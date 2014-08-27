<?php


function recast_views_data() {
  $data = array();

  $data['node']['request_count'] = array(
  'field' => array(
  'title' => t('Requests'),
  'help' => t('Current count of requests for the analysis'),
  'handler' => 'views_handler_request_count',
  'real field' => 'nid',
  'click sortable' => FALSE, //RK just for now until the views handler can be sorted out
  ),
  'sort' => array(
  'handler' => 'views_handler_sort_numeric',
  ),
  );

  $data['node']['request_active_count'] = array(
  'field' => array(
  'title' => t('Active'),
  'help' => t('Current count of recast requests for the analysis that are still active'),
  'handler' => 'views_handler_request_active_count',
  'real field' => 'nid',
  'click sortable' => FALSE, //RK just for now until the views handler can be sorted out
  ),
  'sort' => array(
  'handler' => 'views_handler_sort_numeric',
  ),
  );

  $data['node']['request_completed_count'] = array(
  'field' => array(
  'title' => t('Completed'),
  'help' => t('Current count of recast requests for the analysis that are completed'),
  'handler' => 'views_handler_request_completed_count',
  'real field' => 'nid',
  'click sortable' => FALSE, //RK just for now until the views handler can be sorted out
  ),
  'sort' => array(
  'handler' => 'views_handler_sort_numeric',
  ),
  );

  $data['node']['analysis_actions'] = array(
  'field' => array(
  'title' => t('Actions'),
  'help' => t('Show the action links to view, edit, subscribe and add request for each analysis record in the view'),
  'handler' => 'views_handler_analysis_actions',
  'real field' => 'nid',
  ));

  $data['node']['request_uuid'] = array(
  'field' => array(
  'title' => t('Request UUID'),
  'help' => t('UUID value for this analysis record'),
  'handler' => 'views_handler_request_uuid_value',
  'real field' => 'nid',
  ));

  $data['node']['subscriber_count'] = array(
  'field' => array(
  'title' => t('Subscribers'),
  'help' => t('Count of subscribers to this analysis'),
  'handler' => 'views_handler_subscriber_count',
  'real field' => 'nid',
  'click sortable' => TRUE,
  ));

  return $data;
}

function recast_views_handlers() {
  return array(
  'info' => array(
  'path' => drupal_get_path('module', 'recast'),
  ),
  'handlers' => array(
  'views_handler_request_count' => array(
  'parent' => 'views_handler_field',
  ),
  'views_handler_request_active_count' => array(
  'parent' => 'views_handler_field',
  ),
  'views_handler_request_completed_count' => array(
  'parent' => 'views_handler_field',
  ),
  'views_handler_analysis_actions' => array(
  'parent' => 'views_handler_field',
  ),
  'views_handler_request_uuid_value' => array(
  'parent' => 'views_handler_field',
  ),
  'views_handler_subscriber_count' => array(
  'parent' => 'views_handler_field',
  ),
  ),
  );
}

class views_handler_request_count extends views_handler_field {

  /**
  * Get the value that's supposed to be rendered.
  *
  * @param $values
  *   An object containing all retrieved values.
  * @param $field
  *   Optional name of the field where the value is stored.
  */
  function get_value($values, $field = NULL) {
    // Retrieve the number of requests for for this analysis
    $query = new EntityFieldQuery();
    $result = $query->entityCondition('entity_type', 'node', '=')
    ->entityCondition('bundle', 'recast_request')
    ->propertyCondition('status', 1, '=')
    ->fieldCondition('field_request_analysis', 'target_id', $values->nid, '=', 0)
    ->count()->execute();
    return $result;
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

}

class views_handler_request_active_count extends views_handler_field {

  function get_value($values, $field = NULL) {
    // Retrieve the number of active requests for for this analysis
    $query = new EntityFieldQuery();
    $result = $query->entityCondition('entity_type', 'node', '=')
    ->entityCondition('bundle', 'recast_request')
    ->propertyCondition('status', 1, '=')
    ->fieldCondition('field_request_analysis', 'target_id', $values->nid, '=', 0)
    ->fieldCondition('field_request_status', 'value', 2, '=', 0)
    ->count()->execute();
    return $result;
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

}

class views_handler_request_completed_count extends views_handler_field {

  function get_value($values, $field = NULL) {
    // Retrieve the number of active requests for for this analysis
    $query = new EntityFieldQuery();
    $result = $query->entityCondition('entity_type', 'node', '=')
    ->entityCondition('bundle', 'recast_request')
    ->propertyCondition('status', 1, '=')
    ->fieldCondition('field_request_analysis', 'target_id', $values->nid, '=', 0)
    ->fieldCondition('field_request_status', 'value', 3, '=', 0)
    ->count()->execute();
    return $result;
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

}

class views_handler_analysis_actions extends views_handler_field {


  function render($values) {
    global $user;

    $links =  l('view',"node/{$values->nid}");
    if (user_is_logged_in()) {
      $analysis_node = node_load($values->nid);
      if  (user_access("Analysis: Edit any content") OR $analysis_node->uid == $user->uid) {
        $links .= '&nbsp;|&nbsp;' . l('edit',"node/{$values->nid}/edit");
      }

      // Check and see if user has already subscribed to this analysis - only show if not
      $query = new EntityFieldQuery();
      $entity = $query->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'subscription')
      ->propertyCondition('status', 1, '=')
      ->propertyCondition('uid', $user->uid, '=')
      ->fieldCondition('field_subscription_analysis', 'target_id',  $values->nid, '=', 0)
      ->execute();
      if (isset($entity['node'])) {
        $links .= '&nbsp;|&nbsp;' . l('un-subscribe',"node/{$values->nid}/subscribe/remove/analyses-catalog");
      } else {
        $links .= '&nbsp;|&nbsp;' . l('subscribe',"node/{$values->nid}/subscribe/add");
      }

      $links .= '&nbsp;|&nbsp;' . l('add request',"node/{$values->nid}/request/add") . '&nbsp;';
    }

    return $links;
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

}



class views_handler_request_uuid_value extends views_handler_field {

  function get_value($values, $field = NULL) {
    // Retrieve the vuuid  for this entity propery
    if (isset($values->_field_data['nid_1']['entity']->vuuid)) {
      return $values->_field_data['nid_1']['entity']->vuuid;
    } else {
      return 'missing';
    }
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

}


function recast_views_pre_render(&$view) {
  if ($view->name == 'recast_subscriptions' && $view->current_display == 'page_1')  {
    if (isset($view->args[0])) {
      $nid = $view->args[0];
      $node = node_load($nid);
      if ($node) {
        drupal_set_title($node->title);
      }
    }

  } elseif ($view->name == 'recast_requests' && $view->current_display == 'page_1')  {
    if (isset($view->args[0])) {
      $nid = $view->args[0];
      $node = node_load($nid);
      if ($node) {
        drupal_set_title($node->title);
      }
    }
  }
}

class views_handler_subscriber_count extends views_handler_field {

  function get_value($values, $field = NULL) {
    // Retrieve the number of active requests for for this analysis
    $query = new EntityFieldQuery();
    $result = $query->entityCondition('entity_type', 'node', '=')
    ->entityCondition('bundle', 'subscription')
    ->propertyCondition('status', 1, '=')
    ->fieldCondition('field_subscription_analysis', 'target_id', $values->field_field_request_analysis[0]['raw']['target_id'], '=', 0)
    ->count()->execute();
    return $result;
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

}


