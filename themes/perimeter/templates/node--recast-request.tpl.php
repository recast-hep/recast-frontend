<?php

/**
 * @file
 * Recast's theme implementation to display a node.
 *
 * Available variables:
 * - $title: the (sanitized) title of the node.
 * - $content: An array of node items. Use render($content) to print them all,
 *   or print a subset such as render($content['field_example']). Use
 *   hide($content['field_example']) to temporarily suppress the printing of a
 *   given element.
 * - $user_picture: The node author's picture from user-picture.tpl.php.
 * - $date: Formatted creation date. Preprocess functions can reformat it by
 *   calling format_date() with the desired parameters on the $created variable.
 * - $name: Themed username of node author output from theme_username().
 * - $node_url: Direct url of the current node.
 * - $display_submitted: Whether submission information should be displayed.
 * - $submitted: Submission information created from $name and $date during
 *   template_preprocess_node().
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. The default values can be one or more of the
 *   following:
 *   - node: The current template type, i.e., "theming hook".
 *   - node-[type]: The current node type. For example, if the node is a
 *     "Blog entry" it would result in "node-blog". Note that the machine
 *     name will often be in a short form of the human readable label.
 *   - node-teaser: Nodes in teaser form.
 *   - node-preview: Nodes in preview mode.
 *   The following are controlled through the node publishing options.
 *   - node-promoted: Nodes promoted to the front page.
 *   - node-sticky: Nodes ordered above other non-sticky nodes in teaser
 *     listings.
 *   - node-unpublished: Unpublished nodes visible only to administrators.
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 *
 * Other variables:
 * - $node: Full node object. Contains data that may not be safe.
 * - $type: Node type, i.e. story, page, blog, etc.
 * - $comment_count: Number of comments attached to the node.
 * - $uid: User ID of the node author.
 * - $created: Time the node was published formatted in Unix timestamp.
 * - $classes_array: Array of html class attribute values. It is flattened
 *   into a string within the variable $classes.
 * - $zebra: Outputs either "even" or "odd". Useful for zebra striping in
 *   teaser listings.
 * - $id: Position of the node. Increments each time it's output.
 *
 * Node status variables:
 * - $view_mode: View mode, e.g. 'full', 'teaser'...
 * - $teaser: Flag for the teaser state (shortcut for $view_mode == 'teaser').
 * - $page: Flag for the full page state.
 * - $promote: Flag for front page promotion state.
 * - $sticky: Flags for sticky post setting.
 * - $status: Flag for published status.
 * - $comment: State of comment settings for the node.
 * - $readmore: Flags true if the teaser content of the node cannot hold the
 *   main body content.
 * - $is_front: Flags true when presented in the front page.
 * - $logged_in: Flags true when the current user is a logged-in member.
 * - $is_admin: Flags true when the current user is an administrator.
 *
 * Field variables: for each field instance attached to the node a corresponding
 * variable is defined, e.g. $node->body becomes $body. When needing to access
 * a field's raw values, developers/themers are strongly encouraged to use these
 * variables. Otherwise they will have to explicitly specify the desired field
 * language, e.g. $node->body['en'], thus overriding any language negotiation
 * rule that was previously applied.
 *
 * @see template_preprocess()
 * @see template_preprocess_node()
 * @see template_process()
 */
?>

<div id="node-<?php print $node->nid; ?>" class="<?php print $classes; ?> clearfix"<?php print $attributes; ?>>
  <?php if (!empty($request_status)) { ?>
    <div class="request_status_msg"><?php print $request_status; ?> </div>
  <?php } ?>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Request ID'); ?>:</div><div class="field-items"><?php print $node->title; ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Analysis'); ?>:</div><div class="field-items"><?php print $analysis_title; ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Status'); ?>:</div><div class="field-items"><?php
    global $base_url;
    if(isset($node->field_request_status[LANGUAGE_NONE])) {
    switch ($node->field_request_status[LANGUAGE_NONE][0]['value']) {
      case 0:
       print t('Incomplete');
       break;
     case 1:
       print t('Active');
       break;
      case 2:
       print t('In Progress');
       break;
      case 3:
       print t('Completed');
       break;
      case 9:
       print t('Cancelled');
       break;
      default:
       print t('Unknown');
    }
    ?><?php if ($showcancel) { ?>
      <div class="cancelbutton"><button onclick="document.location='<?php print $base_url; ?>/node/<?php print $node->nid; ?>/request/cancel'"><?php print t('Cancel Request'); ?></button></div>
    <?php
    }
    }
    ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Requester'); ?>:</div><div class="field-items"><?php if (isset($requester_name)) print $requester_name; ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Recast Audience'); ?>:</div><div class="field-items"><?php if (isset($node->field_request_audience[LANGUAGE_NONE])) print $node->field_request_audience[LANGUAGE_NONE][0]['value']; ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Model Name'); ?>:</div><div class="field-items"><?php if (isset($model)) print $model; ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Selected Subscriber(s)'); ?>:</div><div class="field-items"><?php if(isset($subscribers)) print $subscribers; ?></div></div>
  <?php if(isset($request_status_log) AND !empty($request_status_log)) { ?>
    <div class="field-label-inline field clearfix"><div class="field-label"></div><div class="request_subscribers_info"><?php print $request_status_log; ?></div></div>
  <?php } ?>

  <fieldset class="fieldset"><legend><?php print t('Request Description and Potential'); ?></legend>
  <div class="fieldset-wrapper">
  <div class="field clearfix"><div class="field-label" style="width:200px;"><?php print t('Reason for request'); ?>:</div>
    <div class="field-items">
     <?php
      if (isset($node->field_request_reason[LANGUAGE_NONE][0]['value']) AND !empty($node->field_request_reason[LANGUAGE_NONE][0]['value'])) {
        print $node->field_request_reason[LANGUAGE_NONE][0]['value'];
      } else {
        print t('No information available');
      }
     ?>
    </div>
  </div>
  <div class="field clearfix"><div class="field-label" style="width:200px;"><?php print t('Additional Information'); ?>:</div>
    <div class="field-items">
      <?php
      if (isset($node->field_request_info[LANGUAGE_NONE][0]['value']) AND !empty($node->field_request_info[LANGUAGE_NONE][0]['value'])) {
        print $node->field_request_info[LANGUAGE_NONE][0]['value'];
      } else {
        print t('No information available');
      }
      ?>
    </div>
  </div>

  </div>
  </fieldset>

    <?php
    if (isset($content['field_request_parameter_points'])) {
      ?><fieldset class="fieldset"><legend><?php print t('Request Parameter Points'); ?></legend>
      <div class="fieldset-wrapper">
          <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Run Condition'); ?>:</div>
            <div class="field-items"><?php if(isset($run_condition_name)) { ?>
            <div class="recast_edit_runcondition_name"> <?php print $run_condition_name; ?></div><div class="recast_edit_runcondition_desc"> <?php print $run_condition_description; ?></div></div>
            <?php } ?>
          </div>
      <?php
      $parameters = 0;
      foreach ($content['field_request_parameter_points']['#items'] as $num => $parm) {
        if (is_array($parm) AND isset($parm['value'])) {
          $parameters++;
          $parm_point_entity_id = $parm['value'];
          $parm_point_entity = current(entity_load('field_collection_item', array($parm_point_entity_id)));
          ?>
          <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Parameter Point'); print $num +1; ?>:</div><div class="field-items"><?php if (isset($parm_point_entity->field_req_parm_point[LANGUAGE_NONE])) print $parm_point_entity->field_req_parm_point[LANGUAGE_NONE][0]['value']; ?></div></div>
          <div class="clearfix" style="padding:0px 0px 10px 20px;border:0px solid #CCC;">
          <?php
          if (isset($parm_point_entity->field_req_parm_run_condition[LANGUAGE_NONE]) AND $parm_point_entity->field_req_parm_run_condition[LANGUAGE_NONE][0]['value'] > 0 ) {
          foreach ($parm_point_entity->field_req_parm_run_condition[LANGUAGE_NONE] as $condition) {
          $run_condition_entity_id = $condition['value'];
          $parm_run_condition_entity = current(entity_load('field_collection_item', array($run_condition_entity_id)));
          ?>

          <div class="field-label-inline field clearfix">
            <div class="field-label" style="padding-left:160px;width:100px"><?php print t('LHE File'); ?>:</div>
            <div class="field-items" style="width:400px;">&nbsp;<?php if($show_lhe_file AND isset($parm_run_condition_entity->field_request_parm_lhe_file[LANGUAGE_NONE])) { ?><a href="<?php print file_create_url($parm_run_condition_entity->field_request_parm_lhe_file[LANGUAGE_NONE][0]['uri']); ?>">Download File</a><?php } else { ?> <span class="unavailable"> <?php print t('Must subscribe and accept request to download file'); } ?></span></div>
            <div style="clear:both;padding-left:160px;padding-top:0px;">
            <div class="field-label" style="width:100px;"><?php print t('# of events'); ?>:</div>
            <div class="field-items"><?php if(isset($parm_run_condition_entity->field_request_parm_event_count[LANGUAGE_NONE][0]['value'])) print $parm_run_condition_entity->field_request_parm_event_count[LANGUAGE_NONE][0]['value']; ?></div>
            <div class="field-label" style="padding-left:30px;width:200px;"><?php print t('Reference Cross Section'); ?>:</div>
            <div class="field-items"><?php if(isset($parm_run_condition_entity->field_request_parm_ref_xsection[LANGUAGE_NONE][0]['value'])) print $parm_run_condition_entity->field_request_parm_ref_xsection[LANGUAGE_NONE][0]['value']; ?>&nbsp;(fb)</div>
            </div>
          </div>
          <?php
          }
          }
          ?> </div> <?php
        }
      }
      if ($parameters == 0) {
          print t('Non Defined');
      }
      ?></div></fieldset><?php
    }
  ?>

</div>

<?php if ($show_comments) print render($content['comments']); ?>
<?php print render($variables['comment_form']); ?>
