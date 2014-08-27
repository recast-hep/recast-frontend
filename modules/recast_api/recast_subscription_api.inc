<?php
    // recast_subscription_api.inc
    

     /**
     * Callback for creating subscription resources.
     *
     * @param object $data
     * @return object
     */
    function _recast_subscription_api_create($data) {
      global $base_url;
      //the return for this callback will be the actual uuid of the created subscription
      //technically this should be done in a location header as well.
      
      //so we have to validate that the incoming data
      //has all of the right keys
      //if not, we return a proper error code.
      //services_error('Missing recast subscription attribute ....', 406);
      $required_keys =  array(
        'analysis-uuid',
        'username',
        'subscription-type',  // provider|observer
      //RK -- removed the ability to mark this as authoritative for phase 1
        //'authoritative',   //0|1
        'requirements',  //text
        'notifications' //all or some of recast_requests|recast_responses|new_subscribers
      );

      foreach($required_keys as $key) {
        if(!array_key_exists($key, $data)) {
          return services_error('Missing recast subscription attribute ' . $key, 406);
        }
      }
      
      //now lets get the userID from the username
      $user = user_load_by_name($data['username']);
      if($user === FALSE) {
        return services_error('Incorrect user for recast subscription', 406);
      }
      
      if(intval($data['authoritative']) != 0 && intval($data['authoritative']) != 1) {
        return services_error('Athoritative value must be 1 or 0.', 406);
      }
      
      //now check for the uuid -- it should match and get the NID
      $query = new EntityFieldQuery();
      $entity = $query
      ->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'analysis')
      ->propertyCondition('status', 1)   
      ->propertyCondition('uuid', $data['analysis-uuid'])  
      ->execute();
     
      if(count($entity) != 1) {
        return services_error('Invalid analysis UUID', 406);
      }
      
      //now check if they've already subscribed
      $query = new EntityFieldQuery();
      $analysis_entity = $query
      ->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'subscription')
      ->propertyCondition('status', 1, '=')
      ->propertyCondition('uid', $user->uid, '=')
      ->fieldCondition('field_subscription_analysis', 'target_id',  intval(current($entity['node'])->nid), '=', 0)
      ->execute();
      
      if(count($analysis_entity['node']) > 0) {
        return services_error('You have already subscribed to this analysis.', 406);
      }

      //lets load the analysis node for use later
      $nid = current($entity['node']);
      $nid = $nid->nid;
      $analyis_node = node_load($nid);
      $lang = ($analyis_node->language == '') ? 'und' : $analyis_node->language;
  
      $node = new StdClass();
      $node->language = LANGUAGE_NONE;
      $node->type = 'subscription';
      $node->title = 'Subscription';
      $node->status = 1;
      $node->uid = $user->uid;
      $node->field_recast_audience[$node->language][0]['value'] = $data['audience'];
      $node->field_subscription_analysis[$node->language][0]['target_id'] = intval(current($entity['node'])->nid); 

      //RK -- need to create a delta here
      $note = explode(",", $data['notifications']);
      foreach($note as $s) {
        $node->field_subscription_notifications[$node->language][]['value'] = $s;
      }
      $node->field_subscription_type[$node->language][0]['value'] = $data['subscription-type'];
      //RK -- removed the ability to mark this as authoritative for phase 1
      //$node->field_subscription_authoritative[$node->language][0]['value'] = intval($data['authoritative']);
      $node->field_subscription_requirements[$node->language][0]['value'] = $data['requirements'];
      node_save($node);
      
      //now send out the email notifying people that this is a new subscription
      $values['sender'] = $user;
      $values['sender']->name = $user->name;
      $values['sender']->mail = $user->mail;
      $values['subject'] = "New Analysis Subscription";
      $values['node'] = $analyis_node;
      $values['linked-url'] =  l($analyis_node->title,"node/{$analyis_node->nid}");
      $values['subscriber_type'] = $data['subscription-type'];
      drupal_mail('recast', 'new_subscriber', $user, language_default(), $values);
      
      drupal_add_http_header('URI', $base_url . '/api/recast-subscription/' . $node->uuid);
      return $node->uuid;
    }
    
    
    /**
     * Callback for updating recast api resources.
     *
     * @param int $id
     * @param object $data
     * @return object
     */
    function _recast_subscription_api_update($id, $data) {
      global $base_url;
      
      $required_keys =  array(
        'analysis-uuid',
        'username',
        'subscription-type',  // provider|observer
        'authoritative',   //0|1
        'requirements',  //text
        'notifications' //all or some of recast_requests|recast_responses|new_subscribers
      );
      
      foreach($required_keys as $key) {
        if(!array_key_exists($key, $data)) {
          return services_error('Missing recast subscription attribute ' . $key, 406);
        }
      }
      
      //now lets get the userID from the username
      $user = user_load_by_name($data['username']);
      if($user === FALSE) {
        return services_error('User authentication rejected for recast subscription', 406);
      }
      
      $query = new EntityFieldQuery();
      $entity = $query
      ->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'subscription')
      ->propertyCondition('status', 1)      
      ->propertyCondition('uuid', $id)              
      ->execute();
      $nid = intval((current($entity['node'])->nid));
      if($nid == 0) {
        return services_error('Invalid recast subscription uuid', 406);
      }
      $node = node_load($nid);
      $node->field_recast_audience[$node->language][0]['value'] = $data['audience'];
      $node->field_subscription_analysis[$node->language][0]['target_id'] = intval(current($entity['node'])->nid); 

      //RK -- need to create a delta here
      $note = explode(",", $data['notifications']);
      foreach($note as $s) {
        $node->field_subscription_notifications[$node->language][]['value'] = $s;
      }
      $node->field_subscription_type[$node->language][0]['value'] = $data['subscription-type'];
      $node->field_subscription_authoritative[$node->language][0]['value'] = intval($data['authoritative']);
      $node->field_subscription_requirements[$node->language][0]['value'] = $data['requirements'];
      node_save($node);
      drupal_add_http_header('URI', $base_url . '/api/recast-subscription/' . $node->uuid);
      return $node->uuid;
  }
    
    
    /**
     * Callback for retrieving recast subscription 
     *
     * @param int $uuid
     * @return array
     */
    function _recast_subscription_api_retrieve($uuid) {
      $query = new EntityFieldQuery();
      $subscriptions = $query
      ->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'subscription')
      ->propertyCondition('status', 1)      
      ->propertyCondition('uuid', $uuid)              
      ->execute();
      $nodes = node_load_multiple(array_keys($subscriptions['node']));
      foreach($nodes as $n) {
        $lang = $n->language;
        if($lang == '') $lang = 'und';
        $u = user_load($n->uid); //get the user who subscriptioned this thing!
        $subscribers = array();
        foreach($n->field_subscription_subscribers[$lang] as $s) {
          $subscriber_user = user_load($s['uid']);
          $subscribers[] = $subscriber_user->name;
        }
       
        $analysis = node_load($n->field_subscription_analysis[$lang][0]['target_id']); //have to fetch off the uuid for the analysis node
        $arr[] = array(
          'uuid' => $n->uuid,
          'title' => $n->title,
          'subscriptionor' => $u->name,
          'status' => $n->field_subscription_status[$lang][0]['value'],
          'analysis-uuid' => $analysis->uuid,
          'audience' => $n->field_recast_audience[$lang][0]['value'],
          'subscribers' => implode(',',$subscribers),
          'predefined-model' => $n->field_subscription_model[$lang][0]['value'],
          'status' => $n->field_subscription_status[$lang][0]['value'],
          'new-model-information' => $n->field_subscription_new_model[$lang][0]['value'],
          'reason-for-subscription' => $n->field_subscription_reason[$lang][0]['value'],
          'additional-information' => $n->field_subscription_info[$lang][0]['value'],
          'model-type' => $n->field_subscription_model_type[$lang][0]['value'],
          'authoritative' => ($n->field_subscription_authoritative[$lang][0]['value'] == 1) ? 'true' : 'false',
        );
      }
      return $arr;
    }
    
    
    /**
     * Callback for deleting recast analysis api resources.
     *
     * @param int $id
     * @param object $data 
     * @return object
     */
    function _recast_subscription_api_delete($uuid, $username) {
      if($username == '') {
        return services_error('Missing username attribute', 406);
      }
      $user = user_load_by_name($username);
      
      $query = new EntityFieldQuery();
      $entity = $query
      ->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'subscription')
      ->propertyCondition('status', 1)      
      ->propertyCondition('uuid', $uuid)              
      ->execute();
      $nid = current($entity['node']);
      $node = node_load($nid->nid);
     
      if($node->uid != $user->uid) {
        return services_error('Owner of subscription does not match username', 406);
      } 
      else {
        node_delete($nid->nid);
      }
    }
    
    
    
    /**
     * Callback for listing items.  This callback will retrieve all of the subscriptions in the system
     * usage: http://domain/api/recast-analysis/?page=.....  default is 0
     *
     * @param int $page
     * @return array
     */
    function _recast_subscription_api_index($page, $pagesize, $username) {
      $start = $page * $pagesize;
      $query = new EntityFieldQuery();
      $subscriptions = $query
      ->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'subscription')
      ->propertyCondition('status', 1)                    
      ->range($page,$pagesize);
      
      if($username != '') {
        $filteruser = user_load_by_name($username);
        $query->propertyCondition('uid', $filteruser->uid);
      }
      
      $subscriptions = $query->execute();

      $nodes = node_load_multiple(array_keys($subscriptions['node']));
      foreach($nodes as $n) {
        $lang = $n->language;
        if($lang == '') $lang = 'und';
        $u = user_load($n->uid);
        $analysis_node = node_load($n->field_subscription_analysis[$lang][0]['target_id']);
        $arr[] = array(
          'uuid' => $n->uuid,
          'title' => $n->title,
          'subscriber' => $u->name,
          'analysis' => $analysis_node->title,
          'analysis-uuid' => $analysis_node->uuid,
          'authoritative' => ($n->field_subscription_authoritative[$lang][0]['value'] == 1) ? 'true' : 'false',
        );
      }
      return $arr;
    }
    
    
    
    