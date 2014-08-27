<?php
    // recast_analysis_api.inc
    

/**
     * Callback for creating an analysis resource.
     *
     * @param object $data
     * @return string
     */
    function _recast_analysis_api_create($data) {
      //we will require all data items to be entered and as such, we will test them all here:
      $required_keys =  array(
        'username',
        'title',
        'collaboration',
        'e_print',
        'journal',
        'doi',
        'inspire_url',
        'description',
      );
      
      foreach($required_keys as $key) {
        if(!array_key_exists($key, $data)) {
          return services_error('Missing recast analysis attribute ' . $key, 406);
        }
      }
      
      $x = field_info_field('field_analysis_collaboration');
      $collaborations = ($x['settings']['allowed_values']);
            
      if(!in_array($data['collaboration'], $collaborations)) {
        $collaborations = implode(',', $collaborations);
        return services_error('Invalid collaboration.  Only valid collaborations are '. $collaborations, 406);
      }
      
      $usr = user_load_by_name($data['username']);
      if($usr === FALSE) {
        return services_error('Invalid user', 406);
      }
      
      //if we made it this far, we're golden!  Time to create the analysis
      $node = new StdClass();
      $node->language = LANGUAGE_NONE;
      $node->type = 'analysis';
      $node->status = 1;
      $node->uid = $usr->uid;
      $node->title = $data['title'];
      $node->field_analysis_collaboration[$node->language][0]['value'] = $data['collaboration'];
      $node->field_analysis_journal[$node->language][0]['value'] = $data['journal'];
      $node->field_analysis_doi[$node->language][0]['value'] = $data['doi'];
      $node->field_analysis_ownership[$node->language][0]['value'] = 0;
      $node->field_analysis_owner[$node->language][0]['value'] = '';
      $node->field_analysis_inspire[$node->language][0]['value'] = $data['inspire_url'];
      $node->field_analysis_eprint[$node->language][0]['value'] = $data['e_print'];
      $node->field_analysis_description[$lang][0]['value'] = $data['description'];
      node_save($node);
      drupal_add_http_header('URI', $base_url . '/api/recast-analysis/' . $node->uuid);
      return $node->uuid;
    }
    
    /**
     * Callback for adding a run condition to an analysis.
     *
     * @param int $uuid
     * @param object $data
     */
    function _recast_analysis_api_add_run_condition($uuid, $data) {
      $required_keys =  array(
        'username',
        'name',
        'description',
      );
      foreach($required_keys as $key) {
        if(!array_key_exists($key, $data)) {
          return services_error('Missing recast run condition attribute ' . $key, 406);
        }
      }
      $query = new EntityFieldQuery();
      $entity = $query
      ->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'analysis')
      ->propertyCondition('status', 1)   
      ->propertyCondition('uuid', $uuid)  
      ->execute();
 
      $nid = intval((current($entity['node'])->nid));
      if($nid == 0) {
        return services_error('Invalid recast analysis uuid', 406);
      }
      $node = node_load($nid);
      $usr = user_load_by_name($data['username']);
      if($usr === FALSE) {
        return services_error('User authentication rejected for recast analysis', 406);
      }
      if($node->uid != $usr->uid) {
        return services_error('User mismatch', 406);
      }
      //ok.. if we get THIS far, we have data.  let's add a run condition.
      $f = entity_create('field_collection_item', array('field_name' => 'field_run_condition'));
      $f->setHostEntity('node', $node);
      $f->field_run_condition_name[$node->language][]['value'] = $data['name'];           
      $f->field_run_condition_description[$node->language][]['value'] = $data['description'];
      $f->save();
    }
    
    
    /**
     * Callback for retrieving recast analysis 
     *
     * @param int $uuid
     * @return array
     */
    function _recast_analysis_api_retrieve($uuid) {
      $query = new EntityFieldQuery();
      $entity = $query
      ->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'analysis')
      ->propertyCondition('status', 1)   
      ->propertyCondition('uuid', $uuid)  
      ->execute();
 
      $nodes = node_load_multiple(array_keys($entity['node']));
      foreach($nodes as $n) {
        $query = new EntityFieldQuery();
        $requests = $query
        ->entityCondition('entity_type', 'node', '=')
        ->entityCondition('bundle', 'recast_request')
        ->fieldCondition('field_request_analysis', 'target_id' , $n->nid)
        ->propertyCondition('status', 1)                    
        ->count()->execute();

        $lang = $n->language;
        if(isset($uuid) && $uuid != '') {
          //create run conditions array
          $runc = array();
          $rc = entity_load('field_collection_item', array($n->field_run_condition[$lang][0]['value']));
          if(isset($rc) && $rc !='') {
            foreach($rc as $entity_id => $obj) {
              $runc[] = array(
                'run_condition_internal_id' => $entity_id,
                'run_condition' => $obj -> field_run_condition_name[$lang][0]['value'],
                'run_condition_description' => $obj -> field_run_condition_description[$lang][0]['value'],
              
              );
            }
          }
          $arr[] = array(
            'uuid' => $n->uuid,
            'title' => $n->title,
            'number_of_requests' => $requests,
            'collaboration' => $n->field_analysis_collaboration[$lang][0]['value'],
            'journal' => $n->field_analysis_journal[$lang][0]['value'],
            'doi' => $n->field_analysis_doi[$lang][0]['value'],
            'description' => $n->field_analysis_description[$lang][0]['value'],
            'ownership' => ($n->field_analysis_ownership[$lang][0]['value'] == 0) ? t('Unowned') : t('Owned'),
            'owner' => $n->field_analysis_owner[$lang][0]['value'], 
            'inspire' => $n->field_analysis_inspire[$lang][0]['value'], 
            'eprint' => $n->field_analysis_eprint[$lang][0]['value'], 
            'created_by' => $n->name, 
              //now the following are the run conditions
            'run_conditions' => $runc
          );
        }
        else {  //this condition **probably** should never happen... I like to have a fallback here anyways
          $arr[] = array(
              'uuid' => $n->uuid,
              'title' => $n->title,
              'number_of_requests' => $requests,
          );
        }
      }
      return $arr;
    }
    
    
    /**
     * Callback for listing items.  This callback will retrieve all of the analyses in the system
     * usage: http://domain/api/recast-analysis/?page=.....  default is 0
     *
     * @param int $page
     * @return array
     */
    function _recast_analysis_api_index($page) {
      $arr = array();

      $query = new EntityFieldQuery();
      $entity = $query
      ->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'analysis')
      ->propertyCondition('status', 1)   
      ->execute();
 
      $nodes = node_load_multiple(array_keys($entity['node']));
      foreach($nodes as $n) {
        $query = new EntityFieldQuery();
        $requests = $query
        ->entityCondition('entity_type', 'node', '=')
        ->entityCondition('bundle', 'recast_request')
        ->fieldCondition('field_request_analysis', 'target_id' , $n->nid)
        ->propertyCondition('status', 1)                    
        ->count()->execute();
        $lang = $n->language;
        $arr[] = array(
          'uuid' => $n->uuid,
          'title' => $n->title,
          'number_of_requests' => $requests,
          'collaboration' => $n->field_analysis_collaboration[$lang][0]['value'],
        );
      }
      return $arr;
    }
    
    
    
    