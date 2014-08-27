<?php
// recast_request_api.inc


/**
 * Callback for creating request resources.
 *
 * @param object $data
 * @return object
 */
function _recast_request_api_create($data) {
  global $base_url;
  //the return for this callback will be the actual uuid of the created request
  //technically this should be done in a location header as well.

  //so we have to validate that the incoming data
  //has all of the right keys
  //if not, we return a proper error code.
  //services_error('Missing recast request attribute ....', 406);
  $required_keys =  array(
        'analysis-uuid',
        'username',
        'title',
        'audience',
        'subscribers',
        'activate',
        'predefined-model',
        'reason-for-request',
        'model-type',
  );

  foreach($required_keys as $key) {
    if(!array_key_exists($key, $data)) {
      return services_error('Missing recast request attribute ' . $key, 406);
    }
  }

  //now lets get the userID from the username
  $usr = user_load_by_name($data['username']);
  if($usr === FALSE) {
    return services_error('Incorrect user for recast request', 406);
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

  $node = new StdClass();
  $node->language = LANGUAGE_NONE;
  $node->type = 'recast_request';
  $node->status = 1;
  $node->uid = $usr->uid;
  $node->title = $data['title'];
  $node->field_recast_audience[$node->language][0]['value'] = $data['audience'];
  $node->field_request_analysis[$node->language][0]['target_id'] = intval(current($entity['node'])->nid);
  //our assumption here is that the list of subscribers should probably be the system usernames which would be comma separated.
  $subscribers = explode(",", $data['subscribers']);
  $subcount = 0;
  foreach ($subscribers as $sub) {
    $sub = trim($sub);
    $usr = user_load_by_name($sub);
    if(intval($usr->uid) > 0) {
      $node->field_request_subscribers[$node->language][$subcount]['uid'] = $usr->uid;
      $subcount +=1;
    }
  }
  $node->field_request_model[$node->language][0]['value'] = $data['predefined-model'];  //model1, model2, model2, model4

  //RK -- disabled the status field update
  //$node->field_request_status[$node->language][0]['value'] = $data['status'];

  if($node->field_request_status[$node->language][0]['value'] == 0 && $data['activate'] == 1) {
    $node->field_request_status_log[$node->language][0]['value'] = format_date(time(),'medium') . ' - Activated';
    $node->field_request_status[$node->language][0]['value'] = 1;
  }


  $node->field_request_new_model[$node->language][0]['value'] = $data['new-model-information'];
  $node->field_request_reason[$node->language][0]['value'] = $data['reason-for-request'];
  $node->field_request_info[$node->language][0]['value'] = $data['additional-information'];
  $node->field_request_model_type[$node->language][0]['value'] = $data['model-type'];  //defined or new

  node_save($node);

  $values['sender'] = $usr;
  $values['subject'] = "New RECAST Request";
  $values['node'] = $node;
  $values['linked-url'] =  l('RECAST Request',"node/{$node->nid}");
  drupal_mail('recast', 'new_recast', $usr, language_default(), $values);

  drupal_add_http_header('URI', $base_url . '/api/recast-request/' . $node->uuid);
  return $node->uuid;
}


function _recast_request_api_add_parameter_point($uuid, $username, $parameter_point, $number_of_events, $cross_sections, $filename, $data) {
  global $base_path;
  $query = new EntityFieldQuery();
  $entity = $query
  ->entityCondition('entity_type', 'node', '=')
  ->entityCondition('bundle', 'recast_request')
  ->propertyCondition('status', 1)
  ->propertyCondition('uuid', $uuid)
  ->execute();

  $nid = current($entity['node']);
  $nid = $nid->nid;
  if($nid == 0) {
    return services_error('Invalid recast request uuid', 406);
  }
  $node = node_load($nid);
  $lang = ($node->language == '') ? 'und' : $node->language;

  $ext = explode(".",$filename);
  $ext = $ext[count(ext)];

  //we have to now count the number of parameter points from here
  $point_num = (count($node->field_request_parameter_points[$lang])) + 1;
  $filename = "{$node->title}_parm_point{$point_num}_run_condition1.{$ext}";
  $dest=field_info_instance('field_collection_item','field_request_parm_lhe_file','field_req_parm_run_condition');
  $dest = $dest['settings']['file_directory'];
  $path =  realpath(".") . '/' . variable_get('file_public_path', conf_path() . '/files') . "/$dest/" ;
  $path = str_replace("\\","/", $path);

  $file = fopen($path . $filename, "w");
  fwrite($file, $data);
  fclose($file);
  $placeholder_file = file_get_contents($path . $filename);
  $file = file_save_data($placeholder_file, 'public://' . $filename, FILE_EXISTS_RENAME);

  $f = entity_create('field_collection_item', array('field_name' => 'field_request_parameter_points'));
  $f->setHostEntity('node', $node);
  $f->field_req_parm_point[$lang][0]['value'] = $parameter_point;

  $g = entity_create('field_collection_item', array('field_name' => 'field_req_parm_run_condition'));
  $g->setHostEntity('entity', $f);
  $g->field_request_parm_event_count[$lang][]['value'] = $number_of_events;
  $g->field_request_parm_ref_xsection[$lang][]['value'] = $cross_sections;
  $g->field_request_parm_lhe_file[$lang][]= array(
    'fid' => $file->fid,
        'title' => basename($file->filename),
        'filename' => $file->filename,
        'filepath' => $file->filepath,
        'filesize' => $file->filesize,
        'mimetype' => $mime,
        'description' => basename($file->filename),
        'list' => 1,
        'display' => 1
    );

  $f->field_req_parm_run_condition[$lang][]['value'] = $g;
  $f->save();

  drupal_add_http_header('URI', $base_url . '/api/recast-request/' . $node->uuid);
  return $node->uuid;
}


/**
 * Callback for updating recast api resources.
 *
 * @param int $id
 * @param object $data
 * @return object
 */
function _recast_request_api_update($id, $data) {
  global $base_url;
  $required_keys =  array(
        'analysis-uuid',
        'username',
        'title',
        'audience',
        'subscribers',
        'predefined-model',
        'reason-for-request',
        'model-type',
        'activate'
  );

  foreach($required_keys as $key) {
    if(!array_key_exists($key, $data)) {
      return services_error('Missing recast request attribute ' . $key, 406);
    }
  }

  //now lets get the userID from the username
  $usr = user_load_by_name($data['username']);
  if($usr === FALSE) {
    return services_error('User authentication rejected for recast request', 406);
  }

  $query = new EntityFieldQuery();
  $entity = $query
  ->entityCondition('entity_type', 'node', '=')
  ->entityCondition('bundle', 'recast_request')
  ->propertyCondition('status', 1)
  ->propertyCondition('uuid', $id)
  ->execute();
  $nid = intval((current($entity['node'])->nid));
  if($nid == 0) {
    return services_error('Invalid recast request uuid', 406);
  }
  $node = node_load($nid);


  //so, lets only let this happen if this is in incomplete status phase
  if($node->field_request_status[$node->language][0]['value'] != 0) {
    return services_error('This request is not in the INCOMPLETE status phase.  Update rejected.', 406);
  }

  $node->uid = $usr->uid;
  $node->title = $data['title'];
  $node->field_recast_audience[$node->language][0]['value'] = $data['audience'];
  $node->field_request_analysis[$node->language][0]['target_id'] = intval(current($entity['node'])->nid);
  //our assumption here is that the list of subscribers should probably be the system usernames which would be comma separated.
  $subscribers = explode(",", $data['subscribers']);
  $subcount = 0;
  foreach ($subscribers as $sub) {
    $sub = trim($sub);
    $usr = user_load_by_name($sub);
    if(intval($usr->uid) > 0) {
      $node->field_request_subscribers[$node->language][$subcount]['uid'] = $usr->uid;
      $subcount +=1;
    }
  }
  $node->field_request_model[$node->language][0]['value'] = $data['predefined-model'];  //model1, model2, model2, model4
  $node->field_request_new_model[$node->language][0]['value'] = $data['new-model-information'];
  $node->field_request_reason[$node->language][0]['value'] = $data['reason-for-request'];
  $node->field_request_info[$node->language][0]['value'] = $data['additional-information'];
  $node->field_request_model_type[$node->language][0]['value'] = $data['model-type'];  //defined or new

  if($node->field_request_status[$node->language][0]['value'] == 0 && $data['activate'] == 1) {
    $node->field_request_status_log[$node->language][0]['value'] = format_date(time(),'medium') . ' - Activated';
    $node->field_request_status[$node->language][0]['value'] = 1;
  }

  node_save($node);
  drupal_add_http_header('URI', $base_url . '/api/recast-request/' . $id);

  return $id;
}


/**
 * Callback for retrieving recast analysis
 *
 * @param int $uuid
 * @return array
 */
function _recast_request_api_retrieve($uuid) {
  $query = new EntityFieldQuery();
  $requests = $query
  ->entityCondition('entity_type', 'node', '=')
  ->entityCondition('bundle', 'recast_request')
  ->propertyCondition('status', 1)
  ->propertyCondition('uuid', $uuid)
  ->execute();
  $nodes = node_load_multiple(array_keys($requests['node']));
  foreach($nodes as $n) {
    $lang = $n->language;
    if($lang == '') $lang = 'und';
    $u = user_load($n->uid); //get the user who requested this thing!
    $subscribers = array();
    if(isset($n->field_request_subscribers[$lang])) {
      foreach($n->field_request_subscribers[$lang] as $s) {
        $subscriber_user = user_load($s['uid']);
        $subscribers[] = $subscriber_user->name;
      }
    }

    // here we generate the entity output for the field set for request parms
    $parms = array();
    if(isset($n->field_request_parameter_points[$lang])) {
      foreach ($n->field_request_parameter_points[$lang] as $key => $parm_reference) {
        $p = entity_load('field_collection_item', array($parm_reference['value']));
        //so now $p holds the current entity field set collection
        //each run condition holds its own array of entities too.  what fun!  drupal overhead galore here
        $run_condition_points = array();
        foreach($p[$parm_reference['value']]->field_req_parm_run_condition[$lang] as $rc_key => $rc_ref) {
          $rc = entity_load('field_collection_item', array($rc_ref['value']));
          $run_conditions_points[$rc_key] = array (
               //'run-condition-unique-key' => $rc_ref['value'],
               'lhe-file' => file_create_url($rc[$rc_ref['value']]->field_request_parm_lhe_file[$lang][0]['uri']),
               //'field_request_parm_run_condition' => $rc[$rc_ref['value']]->field_request_parm_run_condition[$lang][0]['value'],
               'number-of-events' => $rc[$rc_ref['value']]->field_request_parm_event_count[$lang][0]['value'],
               'reference-x-section' => $rc[$rc_ref['value']]->field_request_parm_ref_xsection[$lang][0]['value'],
          );
        }
        $parms['parameter-' . $key] = array (
             'point-name' => $p[$parm_reference['value']]->field_req_parm_point[$lang][0]['value'],
             'run-condition' => $run_conditions_points
        );
      }
    }
    $analysis = node_load($n->field_request_analysis[$lang][0]['target_id']); //have to fetch off the uuid for the analysis node
    $arr[] = array(
        'uuid' => $n->uuid,
        'title' => $n->title,
        'requestor' => $u->name,
        'status' => $n->field_request_status[$lang][0]['value'],
        'analysis-uuid' => $analysis->uuid,
        'audience' => @$n->field_recast_audience[$lang][0]['value'],
        'subscribers' => implode(',',$subscribers),
        'predefined-model' => @$n->field_request_model[$lang][0]['value'],
        'status' => $n->field_request_status[$lang][0]['value'],
        'new-model-information' => @$n->field_request_new_model[$lang][0]['value'],
        'reason-for-request' => @$n->field_request_reason[$lang][0]['value'],
        'additional-information' => @$n->field_request_info[$lang][0]['value'],
        'model-type' => $n->field_request_model_type[$lang][0]['value'],
        'parameter-points' => $parms
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
function _recast_request_api_delete($uuid, $usrname) {
  if($usrname == '') {
    return services_error('Missing username attribute', 406);
  }
  $usr = user_load_by_name($usrname);

  $query = new EntityFieldQuery();
  $entity = $query
  ->entityCondition('entity_type', 'node', '=')
  ->entityCondition('bundle', 'recast_request')
  ->propertyCondition('status', 1)
  ->propertyCondition('uuid', $uuid)
  ->execute();
  $nid = current($entity['node']);
  $node = node_load($nid->nid);

  if($node->uid != $usr->uid) {
    return services_error('Owner of request does not match username', 406);
  }
  else {
    node_delete($nid->nid);
  }
}

/**
 * Callback for accepting a request.
 *
 * @param string $uuid
 * @param string $usrname
 * @return array
 */
function _recast_request_api_accept($uuid, $usrname) {
  //we are actually creating a recast response here.
  $query = new EntityFieldQuery();
  $entity = $query
  ->entityCondition('entity_type', 'node', '=')
  ->entityCondition('bundle', 'recast_request')
  ->propertyCondition('status', 1)
  ->propertyCondition('uuid', $uuid)
  ->execute();
  $nid = current($entity['node']);
  $nid = $nid->nid;

  if($nid == 0) {
    return services_error('Invalid recast request uuid', 406);
  }
  $request = node_load($nid);
  $request_lang = ($request->language =='') ? 'und' : $request->language;

  //If the request is canceled, do not allow them to accept the request
  if($request->field_request_status[$request_lang][0]['value'] == 9 || $request->field_request_status[$request_lang][0]['value'] == 0) {
    return services_error('This request is incomplete or has been canceled.  You cannot accept an incomplete or canceled request.', 406);
  }

  $usr = user_load_by_name($usrname);
  if(!isset($usr)) {
    return services_error('Invalid Username', 406);
  }

  //now, if the user specified is not a part of the subscribers, don't accept them
  //lets see if this user is first subscribed to this analysis
  $query = new EntityFieldQuery();
  $subscription_entity = $query
  ->entityCondition('entity_type', 'node', '=')
  ->entityCondition('bundle', 'subscription')
  ->propertyCondition('status', 1)
  ->propertyCondition('uid', $usr->uid, '=')
  ->fieldCondition('field_subscription_analysis', 'target_id',  $request->field_request_analysis[$request_lang][0]['target_id'], '=', 0)
  ->execute();
  $subscription_node_id = @current($subscription_entity['node'])->nid;

  if( intval($subscription_node_id) == 0 ) {
    return services_error('You are not subscribed to this analysis.  You cannot accept a canceled request.', 406);
  }
  else {
    $subscription_node = node_load($subscription_node_id);
    $subscription_language = ($subscription_node->language == '') ? 'und' : $subscription_node->language;
    $authoritative = false;
    if(isset($subscription_node->field_subscription_authoritative[$subscription_language][0]['value'])) {
      $authoritative = ($subscription_node->field_subscription_authoritative[$subscription_language][0]['value'] == 1) ? true : false;
    }

  }

  //at this point, we are subscribed, but now lets see if they are in the subscriber list
  //if field_request_audience is all, then we're good to go.  therefore check the reverse.
  switch ($request->field_request_audience[$request_lang][0]['value'] ) {
    case 'all':
        //just a catch-all in here.  if it is all, then we're OK to proceed.
      break;

    case 'authoritative':
      if($authoritative != true) {
        return services_error('This request is for authoritative subscribers only.', 406);
      }
      break;

    case 'selective':
      //lets see if we are listed as one of the selected subscribers...
      $selected_subscriber = false;
      foreach($request->field_request_subscribers[$request_lang] as $key => $val) {
        if($usr->id == $val['uid']) $selected_subscriber = true;
      }
      if(!$selected_subscriber) {
        return services_error('You are not on the list of selected subscribers to respond to this request.', 406);
      }
      break;
  }

  //now check if there already is a response that this person responded to
  $query = new EntityFieldQuery();
  $entity = $query
  ->entityCondition('entity_type', 'node', '=')
  ->entityCondition('bundle', 'recast_response')
  ->propertyCondition('status', 1, '=')
  ->propertyCondition('uid', $usr->uid, '=')
  ->fieldCondition('field_result_request', 'target_id',  $nid, '=', 0)
  ->execute();

  if(count($entity['node']) > 0) {
    return services_error('You have already accepted this request.', 406);
  }

  $node = new StdClass();
  $node->language = LANGUAGE_NONE;
  $node->type = 'recast_response';
  $node->uid = $usr->uid;
  $node->title = 'Recast Response for Request #' . $request->title;
  $node->field_result_request[$node->language][0]['target_id'] = $request->nid;

  node_save($node);
  drupal_add_http_header('URI', $base_url . '/api/recast-response/' . $node->uuid);
  return array('response-uuid' => $node->uuid);
}


/**
 * Callback for listing items.  This callback will retrieve all of the requests in the system
 * usage: http://domain/api/recast-analysis/?page=.....  default is 0
 *
 * @param int $page
 * @param int $pagesize
 * @param string $usrname
 * @param string type
 * @return array
 */
function _recast_request_api_index($page, $pagesize, $usrname, $type) {
  $start = $page * $pagesize;
  $query = new EntityFieldQuery();
  //we have to determine if the type is set to accepted
  if($type == 'accepted') {
    if($usrname == '') {
      return services_error('You must provide your username for this query', 406);
    }
    $filteruser = user_load_by_name($usrname);

    $requests = $query
    ->entityCondition('entity_type', 'node', '=')
    ->entityCondition('bundle', 'recast_response')
    ->propertyCondition('status', 1)
    ->range($page,$pagesize);
    $query->propertyCondition('uid', $filteruser->uid);

    $requests = $query->execute();
    //here we have to bring back all of the associated requests along with the
    //response uuid
    $nodes = node_load_multiple(array_keys($requests['node']));  //fetching RESPONSE nodes
    foreach($nodes as $r) {
      $request_node_id = $r->field_result_request[$lang][0]['target_id'];
      $n = node_load($request_node_id);
      $lang = $r->language;
      if($lang == '') $lang = 'und';
      $u = user_load($n->uid);
      $arr[] = array(
          'response-uuid' => $r->uuid,
          'response-title' => $r->title,
          'uuid' => $n->uuid,
          'title' => $n->title,
          'requestor' => $u->name,
          'status' => $n->field_request_status[$lang][0]['value'],
      );
    }
  }
  else {
    $requests = $query
    ->entityCondition('entity_type', 'node', '=')
    ->entityCondition('bundle', 'recast_request')
    ->propertyCondition('status', 1)
    ->range($page,$pagesize);

    if($usrname != '') {
      $filteruser = user_load_by_name($usrname);
      $query->propertyCondition('uid', $filteruser->uid);
    }

    $requests = $query->execute();
    $nodes = node_load_multiple(array_keys($requests['node']));
    foreach($nodes as $n) {
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
  }

  return $arr;
}



