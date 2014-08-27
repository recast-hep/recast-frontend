<?php


function _recast_response_api_update ($uuid, $fileid, $data) {
  global $base_path;
  $query = new EntityFieldQuery();
  $entity = $query
  ->entityCondition('entity_type', 'node', '=')
  ->entityCondition('bundle', 'recast_response')
  ->propertyCondition('status', 1)
  ->propertyCondition('uuid', $uuid)
  ->execute();

  $nid = current($entity['node']);
  $nid = $nid->nid;
  if($nid == 0) {
    return services_error('Invalid recast response uuid', 406);
  }
  $node = node_load($nid);
  $lang = $node->language == '' ? 'und' : $node->language;
  $usr = user_load($node->uid);

  if($fileid != '') {  //we are updating the TH1 file component of this response
    $fileid = date('Ymdhis') . $fileid;
    $path =  realpath(".") . '/' . variable_get('file_public_path', conf_path() . '/files/temp_recast_api/') ;
    $path = str_replace("\\","/", $path);
    $file = fopen($path . $fileid, "w");
    fwrite($file, $data);
    fclose($file);

    $placeholder_file = file_get_contents($path . $fileid);
    $file = file_save_data($placeholder_file, 'public://' . $fileid, FILE_EXISTS_RENAME);

    $node->field_result_file[$lang][0]= array(
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
    $node->field_result_status[$lang][0]['value'] = 'completed';
    node_save($node);
    //ok, now we have to load the request node and mark it as complete
    $request_node = node_load($node->field_result_request[$lang][0]['target_id']);
    if ($request_node) {
      // Flag request as complete - status code of 3
      $request_node->field_request_status[LANGUAGE_NONE][0]['value'] = RECAST_REQUEST_COMPLETED;
      $request_node->field_request_status_log[LANGUAGE_NONE][0]['value'] .= '<br>' . format_date(time(),'medium') . ' - Completed';
      node_save($request_node);
    }

    //now tell everyone about it
    $values['sender'] = $usr;
    $values['sender']->name = $usr->name;
    $values['sender']->mail = $usr->mail;
    $values['subject'] = "New RECAST Response";
    $values['node'] = $request_node;
    $values['linked-url'] =  l($request_node->title,"node/{$request_node->nid}");
    drupal_mail('recast', 'new_response', $usr, language_default(), $values);
  }
  else { //we are updating the actual data in the response

    //$node->field_total_luminosity[$lang][0]['value'] = filter_xss($data['total_luminosity']);
    $node->field_luminosity_efficiency[$lang][0]['value'] = filter_xss($data['luminosity_efficiency']);
    $node->field_lower1_signal[$lang][0]['value'] = filter_xss($data['lower1_signal']);
    $node->field_upper1_signal[$lang][0]['value'] = filter_xss($data['upper1_signal']);
    $node->field_upper2_signal[$lang][0]['value'] = filter_xss($data['upper2_signal']);
    $node->field_lower2_signal[$lang][0]['value'] = filter_xss($data['lower2_signal']);
    $node->field_result_url[$lang][0]['url'] = filter_xss($data['result_url']);
    node_save($node);
  }
  drupal_add_http_header('URI', $base_url . '/api/recast-response/' . $node->uuid);
  return array('response-uuid' => $node->uuid);
}


function _recast_response_api_index($page, $pagesize, $username) {
  $start = $page * $pagesize;
  $query = new EntityFieldQuery();

  $responses = $query
  ->entityCondition('entity_type', 'node', '=')
  ->entityCondition('bundle', 'recast_response')
  ->propertyCondition('status', 1)
  ->range($page,$pagesize);

  if($username != '') {
    $filteruser = user_load_by_name($username);
    $query->propertyCondition('uid', $filteruser->uid);
  }

  $responses = $query->execute();
  //here we have to bring back all of the associated requests along with the
  //response uuid
  $nodes = node_load_multiple(array_keys($responses['node']));  //fetching RESPONSE nodes
  $arr = array();
  foreach($nodes as $n) {
    if($lang == '') $lang = 'und';
      $u = user_load($n->uid);
      $arr[] = array(
          'title' => $n->title,
          'uuid' => $n->uuid,
          'title' => $n->title,
          'responder' => $u->name,
      );
  }
  return $arr;
}


function _recast_response_api_retrieve($uuid) {
  $start = $page * $pagesize;
  $query = new EntityFieldQuery();

  $responses = $query
  ->entityCondition('entity_type', 'node', '=')
  ->entityCondition('bundle', 'recast_response')
  ->propertyCondition('status', 1)
  ->propertyCondition('uuid', $uuid);

  $responses = $query->execute();
  $nodes = node_load_multiple(array_keys($responses['node']));  //fetching RESPONSE nodes
  $arr = array();
  foreach($nodes as $n) {
    if($lang == '') $lang = 'und';
      $u = user_load($n->uid);
      $arr[] = array(
          'title' => $n->title,
          'uuid' => $n->uuid,
          'title' => $n->title,
          'responder' => $u->name,
          //'total_luminosity' => $n ->field_total_luminosity[$lang][0]['value'],
          'luminosity_efficiency' => $n ->field_luminosity_efficiency[$lang][0]['value'],
          'lower1_signal' => $n ->field_lower1_signal[$lang][0]['value'],
          'upper1_signal' => $n ->field_upper1_signal [$lang][0]['value'],
          'lower2_signal' => $n ->field_lower2_signal[$lang][0]['value'],
          'result_file ' => $n ->field_result_file [$lang][0]['value'],
          'result_url' => $n ->field_result_url[$lang][0]['value'],
          'result_text_file' => $n ->field_result_text_file[$lang][0]['value'],
      );
  }
  return $arr;
}

