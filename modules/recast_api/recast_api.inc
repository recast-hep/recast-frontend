<?php
    // recast_api.inc
    /**
     * Callback for creating item resources.
     *
     * @param object $data
     * @return object
     */
    function _recast_api_create($data) {
      
      return $data;
    }
    
    /**
     * Callback for updating recast api resources.
     *
     * @param int $id
     * @param object $data
     * @return object
     */
    function _recast_api_update($id, $data) {
      $x=print_r($data, true);
      return $id . $x;
    }
    
    /**
     * Callback for retrieving recast api resources.
     *
     * @param int $id
     * @return object
     */
    function _recast_api_retrieve( $uuid) {
      //return recast_api_get_item($id);
      return $type . $uuid;
    }

    
    /**
     * Callback for retrieving recast resource customized based on its type and uuid
     * The output for this callback is customized for each type of return type.
     *
     * @param int $type
     * @param int $uuid
     * @return object
     */
    function _recast_api_retrieve_type_uuid($type, $uuid) {
      //return $type . $uuid;
      
      switch($type) {
        case 'request':
          //echo $uuid;
            $arr = array();
            $query = new EntityFieldQuery();
            $requests = $query
            ->entityCondition('entity_type', 'node', '=')
            ->entityCondition('bundle', 'recast_request')
            ->propertyCondition('status', 1)      
            ->propertyCondition('uuid', $uuid)              
            ->execute();
            $nodes = node_load_multiple(array_keys($requests['node']));
            foreach($nodes as $n) {
              //print_r($n);
              $lang = $n->language;
              if($lang == '') $lang = 'und';
              $u = user_load($n->uid);
              $arr[] = array(
                'uuid' => $n->uuid,
                'title' => $n->title,
                'requestor' => $u->name,
                'status' => $n->field_request_status[$lang][0]['value'],
              );
            }
          break;
          
          
        case 'analysis':
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
            $arr[] = array(
              'uuid' => $n->uuid,
              'title' => $n->title,
              'number_of_requests' => $requests,
              'collaboration' => $n->field_analysis_collaboration[$lang][0]['value'],
            );
          }
          break;
      }
      
      
      return $arr;
    }
    
    
    /**
     * Callback for deleting recast api resources.
     *
     * @param int $id
     * @return object
     */
    function _recast_api_delete($nid) {
      //recast_api_delete_item($id);
      $x=print_r($nid,true);
      return (object)array(
        'delid' => $x,
      );
    }
    
    
    
    /**
     * Callback for listing items.  This callback will retrieve all of the requested types
     * usage: http://domain/api/recast/?type=.....  default is analysis
     *
     * @param int $page
     * @param array $parameters
     * @return array
     */
    function _recast_api_index($type) {
      $arr = array();
      switch($type) {
        
        default:
        case 'analysis':
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
            //print_r($n);
            $lang = $n->language;
            $arr[] = array(
              'uuid' => $n->uuid,
              'title' => $n->title,
              'number_of_requests' => $requests,
              'collaboration' => $n->field_analysis_collaboration[$lang][0]['value'],
            );
          }
          break;
          
        case 'requests':
          $query = new EntityFieldQuery();
          $requests = $query
          ->entityCondition('entity_type', 'node', '=')
          ->entityCondition('bundle', 'recast_request')
          ->propertyCondition('status', 1)                    
          ->execute();

          $nodes = node_load_multiple(array_keys($requests['node']));
          foreach($nodes as $n) {
            //print_r($n);
            $lang = $n->language;
            if($lang == '') $lang = 'und';
            $u = user_load($n->uid);

            $arr[] = array(
              'uuid' => $n->uuid,
              'title' => $n->title,
              'requestor' => $u->name,
              'status' => $n->field_request_status[$lang][0]['value'],
            );
          
          }
          break;
        
        case 'subscriptions':
          $arr = array();
          break;
      }
      
      
      return $arr;
    }
    
    
    
    