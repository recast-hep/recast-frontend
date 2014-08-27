<?php

function perimeter_breadcrumb($links) {

  $links = array();

  // Set breadcrumbs in one central location for the Recast Application
  $path = drupal_get_path_alias($_GET['q']);
  if($path != 'analyses-catalog') {
    $node = menu_get_object('node');
    if ($node AND is_object($node) AND $node->type != 'page') {
      $node_type = $node->type;
      $links[] = l(t($node->title), "node/{$node->nid}");
      if ($node->type == 'recast_request') {
        $analysis_nid = $node->field_request_analysis[LANGUAGE_NONE][0]['target_id'];
        $analysis_node = node_load($analysis_nid);
        $links[] = l(t('List Requests'), "node/{$analysis_nid}/listrequests");
        $links[] = l($analysis_node->title, "node/{$analysis_nid}");
      } elseif ($node->type == 'recast_response') {
        $request_nid = $node->field_result_request[LANGUAGE_NONE][0]['target_id'];
        $request_node = node_load($request_nid);
        $analysis_nid = $request_node->field_request_analysis[LANGUAGE_NONE][0]['target_id'];
        $analysis_node = node_load($analysis_nid);
        $links[] = l(t('Show Results'), "node/{$request_nid}/request/results");
        $links[] = l($request_node->title, "node/{$request_nid}");
        $links[] = l(t('List Requests'), "node/{$analysis_nid}/listrequests");
        $links[] = l($analysis_node->title, "node/{$analysis_nid}");
      }
      $links[] = l(t('Analyses Catalog'), "analyses-catalog");
      $links[] = l(t('Home'), NULL);
      $links = array_reverse($links);
    } else {
      if (strpos($path,'reply')) {
        $request_nid = arg(2);
        $request_node = node_load($request_nid);
        if ($request_node AND $request_node->type == 'recast_request') {
          $analysis_nid = $request_node->field_request_analysis[LANGUAGE_NONE][0]['target_id'];
          $analysis_node = node_load($analysis_nid);
          $links[] = l($request_node->title, "node/{$request_nid}");
          $links[] = l($analysis_node->title, "node/{$analysis_nid}");
        }
        $links[] = l(t('Analyses Catalog'), "analyses-catalog");
        $links[] = l(t('Home'), NULL);
        $links = array_reverse($links);
      }
    }
  } else {
    drupal_set_breadcrumb(array());
  }

  // Set custom breadcrumbs
  drupal_set_breadcrumb($links);

  // Get custom breadcrumbs
  $breadcrumb = drupal_get_breadcrumb();

  // Hide breadcrumbs if only 'Home' exists
  if (count($breadcrumb) > 1) {
    return '<div class="breadcrumb">'. implode(' &raquo; ', $breadcrumb) .'</div>';
  }

}


/**
* Add body classes if certain regions have content.
*/
function perimeter_preprocess_html(&$variables) {
  if (!empty($variables['page']['featured'])) {
    $variables['classes_array'][] = 'featured';
  }

  if (!empty($variables['page']['triptych_first'])
  || !empty($variables['page']['triptych_middle'])
  || !empty($variables['page']['triptych_last'])) {
    $variables['classes_array'][] = 'triptych';
  }

  if (!empty($variables['page']['footer_firstcolumn'])
  || !empty($variables['page']['footer_secondcolumn'])
  || !empty($variables['page']['footer_thirdcolumn'])
  || !empty($variables['page']['footer_fourthcolumn'])) {
    $variables['classes_array'][] = 'footer-columns';
  }

  // Add conditional stylesheets for IE
  drupal_add_css(path_to_theme() . '/css/ie.css', array('group' => CSS_THEME, 'browsers' => array('IE' => 'lte IE 7', '!IE' => FALSE), 'preprocess' => FALSE));
  drupal_add_css(path_to_theme() . '/css/ie6.css', array('group' => CSS_THEME, 'browsers' => array('IE' => 'IE 6', '!IE' => FALSE), 'preprocess' => FALSE));
}

/**
* Override or insert variables into the page template for HTML output.
*/
function perimeter_process_html(&$variables) {
  // Hook into color.module.
  if (module_exists('color')) {
    _color_html_alter($variables);
  }
}

/**
* Override or insert variables into the page template.
*/
function perimeter_process_page(&$variables) {


  // Hook into color.module.
  if (module_exists('color')) {
    _color_page_alter($variables);
  }
  // Always print the site name and slogan, but if they are toggled off, we'll
  // just hide them visually.
  $variables['hide_site_name']   = theme_get_setting('toggle_name') ? FALSE : TRUE;
  $variables['hide_site_slogan'] = theme_get_setting('toggle_slogan') ? FALSE : TRUE;
  if ($variables['hide_site_name']) {
    // If toggle_name is FALSE, the site_name will be empty, so we rebuild it.
    $variables['site_name'] = filter_xss_admin(variable_get('site_name', 'Drupal'));
  }
  if ($variables['hide_site_slogan']) {
    // If toggle_site_slogan is FALSE, the site_slogan will be empty, so we rebuild it.
    $variables['site_slogan'] = filter_xss_admin(variable_get('site_slogan', ''));
  }
  // Since the title and the shortcut link are both block level elements,
  // positioning them next to each other is much simpler with a wrapper div.
  if (!empty($variables['title_suffix']['add_or_remove_shortcut']) && $variables['title']) {
    // Add a wrapper div using the title_prefix and title_suffix render elements.
    $variables['title_prefix']['shortcut_wrapper'] = array(
    '#markup' => '<div class="shortcut-wrapper clearfix">',
    '#weight' => 100,
    );
    $variables['title_suffix']['shortcut_wrapper'] = array(
    '#markup' => '</div>',
    '#weight' => -99,
    );
    // Make sure the shortcut link is the first item in title_suffix.
    $variables['title_suffix']['add_or_remove_shortcut']['#weight'] = -100;
  }

  if (isset($variables['node'])) {
    // If the node type is "blog" the template suggestion will be "page--blog.tpl.php".
    $variables['theme_hook_suggestions'][] = 'page__'. str_replace('_', '--', $variables['node']->type);

    // Hide the title when viewing these RECAST application node types
    if (in_array($variables['node']->type, array('analysis','recast_request'))) {
      $variables['title'] = FALSE;
    }

  }

}

/**
* Implements hook_preprocess_maintenance_page().
*/
function perimeter_preprocess_maintenance_page(&$variables) {
  if (!$variables['db_is_active']) {
    unset($variables['site_name']);
  }
  drupal_add_css(drupal_get_path('theme', 'perimeter') . '/css/maintenance-page.css');
}

/**
* Override or insert variables into the maintenance page template.
*/
function perimeter_process_maintenance_page(&$variables) {
  // Always print the site name and slogan, but if they are toggled off, we'll
  // just hide them visually.
  $variables['hide_site_name']   = theme_get_setting('toggle_name') ? FALSE : TRUE;
  $variables['hide_site_slogan'] = theme_get_setting('toggle_slogan') ? FALSE : TRUE;
  if ($variables['hide_site_name']) {
    // If toggle_name is FALSE, the site_name will be empty, so we rebuild it.
    $variables['site_name'] = filter_xss_admin(variable_get('site_name', 'Drupal'));
  }
  if ($variables['hide_site_slogan']) {
    // If toggle_site_slogan is FALSE, the site_slogan will be empty, so we rebuild it.
    $variables['site_slogan'] = filter_xss_admin(variable_get('site_slogan', ''));
  }
}



/**
* Override or insert variables into the node template.
*/
function perimeter_preprocess_node(&$variables) {
  global $user;
  if ($variables['view_mode'] == 'full' && node_is_page($variables['node'])) {
    $variables['classes_array'][] = 'node-full';

    if ($variables['node']->type == 'recast_request') {
      global $user;

      $variables['showcancel'] = FALSE;

      // Check and see what the request audience is All or Selective
      $user_is_target = recast_user_in_audience($variables['node']);

      switch ($variables['node']->field_request_status[LANGUAGE_NONE][0]['value']) {
        case RECAST_REQUEST_INCOMPLETE:
          if ($variables['node']->uid == $user->uid) {
            $variables['request_status'] = t('This request is not yet active and subscribers have not been notified. The requester can activate the request in the Edit tab');
          } else {
            $variables['request_status'] = t('This request is not active, thus it is not accepting requests. Contact the requester for more information.');
          }
          break;
        case RECAST_REQUEST_ACTIVE:
        case RECAST_REQUEST_INPROGRESS:
          if ($variables['node']->uid == $user->uid) {
            $variables['request_status'] = t('This request is active and locked from further editing');
            $variables['showcancel'] = TRUE;
          } else {
            if (recast_check_subscription($variables['node'])) {
              if (recast_check_subscription($variables['node'],'observer')) {
                $variables['request_status'] = t('You are subscribed to the analysis as an observer.  In order to accept the request, you must be subscribed as a provider.');
              } elseif (!recast_user_in_audience($variables['node'])) {
                $variables['request_status'] = t('You are not a member of the RECAST audience for this request, so you cannot accept the request. Contact requester if you wish to accept request.');
              }
            } else {
              $variables['request_status'] = t('In order to accept this request, you must first subscribe to the analysis as a provider.');
            }
          }
          break;
        case RECAST_REQUEST_COMPLETED:
          $variables['request_status'] = t('This request is completed');
          if ($variables['node']->uid == $user->uid) {
            $variables['showcancel'] = TRUE;
          }
          break;
        case RECAST_REQUEST_CANCELLED:
          $variables['request_status'] = t('This request has been cancelled');
          break;
      }

      if(isset($variables['node']->field_request_new_model[LANGUAGE_NONE]) AND isset($variables['node']->field_request_new_model[LANGUAGE_NONE][0]['value'])) {
        $variables['model'] = $variables['node']->field_request_new_model[LANGUAGE_NONE][0]['value'];
      } else {
        $variables['model'] = $variables['node']->field_request_model[LANGUAGE_NONE][0]['value'];
      }

      $requester = user_load($variables['node']->uid);
      $variables['requester_name'] = l($requester->name,"user/{$requester->uid}");

      $analysis_nid = $variables['node']->field_request_analysis[LANGUAGE_NONE][0]['target_id'];
      $analysis_node = node_load($analysis_nid);

      $variables['analysis_title'] = $analysis_node->title;
      $variables['subscribers'] = '';
      $variables['subscribers_status'] = '';
      if (isset($analysis_node->field_run_condition[$analysis_node->language])) {

        /* Retrieve the Run Condition name and description - for now we only support 1 */
        foreach ($analysis_node->field_run_condition[$analysis_node->language] as $key => $id) {
          $entity_id = intval($id['value']);
          if ($entity_id > 0) {
            $run_condition_entity = current(entity_load('field_collection_item', array($entity_id)));
            $variables['run_condition_name'] = $run_condition_entity->field_run_condition_name[LANGUAGE_NONE][0]['value'];
            $variables['run_condition_description'] = nl2br( $run_condition_entity->field_run_condition_description[LANGUAGE_NONE][0]['value']);
            break;
          }
        }
      }

      /* Test to see if you should show the LHE File
      * Has this user accepted this recast request
      * or is the request owner
      */
      if  (user_access("Recast Request: Edit any content") OR $requester->uid == $user->uid) {
        $variables['show_lhe_file'] = TRUE;
        //$variables['show_comments'] = TRUE;
      } else {
        //$variables['show_comments'] = FALSE;
        $query = new EntityFieldQuery();
        $entity = $query->entityCondition('entity_type', 'node', '=')
        ->entityCondition('bundle', 'recast_response')
        ->propertyCondition('status', 1, '=')
        ->propertyCondition('uid', $user->uid, '=')
        ->fieldCondition('field_result_request', 'target_id',  $variables['node']->nid, '=', 0)
        ->fieldCondition('field_result_status', 'value',  'accepted', '=', 0)
        ->execute();
        if (isset($entity['node'])) {
          $variables['show_lhe_file'] = TRUE;
        } else {
          $variables['show_lhe_file'] = FALSE;
        }
      }

      // Nov 16: Allow all users to now see comments
      // Eenable the add comment feature site for site members
      $variables['show_comments'] = TRUE;

      // Retrieve the subscribers and create user profile links
      $selectedusers = array();
      if ($variables['node']->field_request_audience[LANGUAGE_NONE][0]['value'] == 'selective') {
        foreach ($variables['node']->field_request_subscribers[LANGUAGE_NONE] as $selected_user) {
          $selectedusers[] = $selected_user['uid'];
        }
      }

      $subscription_query = new EntityFieldQuery();
      $subscription_result = $subscription_query->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'subscription')
      ->propertyCondition('status', 1, '=')
      ->fieldCondition('field_subscription_analysis', 'target_id',  $analysis_nid, '=', 0)
      ->execute();
      if ($subscription_result) {
        $subscription_nodes = entity_load('node', array_keys($subscription_result['node']));
        if (is_array($subscription_nodes) AND count($subscription_nodes) > 0) {
          foreach ($subscription_nodes as $subscription) {
            $subscription_user = user_load($subscription->uid);
            if ($variables['node']->field_request_audience[LANGUAGE_NONE][0]['value'] == 'selective') {
              if (in_array($subscription->uid,$selectedusers)) {
                if (!empty($variables['subscribers'])) $variables['subscribers'] .= ',&nbsp;';
                $variables['subscribers'] .= l($subscription_user->name,"user/{$subscription->uid}");
              }
            } else {
              if (!empty($variables['subscribers'])) $variables['subscribers'] .= ',&nbsp;';
              $variables['subscribers'] .= l($subscription_user->name,"user/{$subscription->uid}");
            }
          }
        }
      }

      /* Nov 15 (Blaine): Disabled compiling a status message from the accept/decline changes
       * which are captured in the comments anyways. Show the request_status_log instead which
       * contains the date/times of the request state changes
      */
      /*
      // Retrieve the subscribers request responses if they accepted or declined
      $recastresponse_query = new EntityFieldQuery();
      $recastresponse = $recastresponse_query->entityCondition('entity_type', 'node', '=')
      ->entityCondition('bundle', 'recast_response')
      ->propertyCondition('status', 1, '=')
      //->propertyCondition('uid', $subscription_user->uid, '=')
      ->fieldCondition('field_result_request', 'target_id',  $variables['node']->nid, '=', 0)
      ->execute();

      if ($recastresponse) {
        $response_nodes = entity_load('node', array_keys($recastresponse['node']));
        if (is_array($response_nodes) AND count($response_nodes) > 0) {
          foreach ($response_nodes as $response) {
            $response_user = user_load($response->uid);
            if ($response->field_result_status[LANGUAGE_NONE][0]['value'] == 'accepted') {
              $variables['subscribers_status'] .= '<div><div class="username">' . $response_user->name .'</div> Accepted ' . format_date($response->created) . '</div>';
            } elseif ($response->field_result_status[LANGUAGE_NONE][0]['value'] == 'declined') {
              $variables['subscribers_status'] .= '<div><span class="username">' . $response_user->name .'</span> Declined ' . format_date($response->created) . '</div>';
            } elseif ($response->field_result_status[LANGUAGE_NONE][0]['value'] == 'completed') {
              $variables['subscribers_status'] .= '<div><span class="username">' . $response_user->name .'</span> Completed ' . format_date($response->changed) . '</div>';
            }
          }
        }
      }
      */

      if (isset($variables['node']->field_request_status_log[LANGUAGE_NONE])) {
        $variables['request_status_log'] = $variables['node']->field_request_status_log[LANGUAGE_NONE][0]['value'];
      }

      unset($variables['content']['comments']['comment_form']);
      if (user_is_logged_in()) {
        $variables['comment_form'] = drupal_get_form('comment_node_recast_request_form',(object) array('nid' => $variables['node']->nid));
      }

    }

  }
}

/**
* Override or insert variables into the block template.
*/
function perimeter_preprocess_block(&$variables) {
  // In the header region visually hide block titles.
  if ($variables['block']->region == 'header') {
    $variables['title_attributes_array']['class'][] = 'element-invisible';
  }
}

/**
* Implements theme_menu_tree().
*/
function perimeter_menu_tree($variables) {
  return '<ul class="menu clearfix">' . $variables['tree'] . '</ul>';
}

/**
* Implements theme_field__field_type().
*/
function perimeter_field__taxonomy_term_reference($variables) {
  $output = '';

  // Render the label, if it's not hidden.
  if (!$variables['label_hidden']) {
    $output .= '<h3 class="field-label">' . $variables['label'] . ': </h3>';
  }

  // Render the items.
  $output .= ($variables['element']['#label_display'] == 'inline') ? '<ul class="links inline">' : '<ul class="links">';
  foreach ($variables['items'] as $delta => $item) {
    $output .= '<li class="taxonomy-term-reference-' . $delta . '"' . $variables['item_attributes'][$delta] . '>' . drupal_render($item) . '</li>';
  }
  $output .= '</ul>';

  // Render the top-level DIV.
  $output = '<div class="' . $variables['classes'] . (!in_array('clearfix', $variables['classes_array']) ? ' clearfix' : '') . '">' . $output . '</div>';

  return $output;
}
