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

  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Title'); ?>:</div><div class="field-items"><?php print $node->title; ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Collaboration'); ?>:</div><div class="field-items"><?php if (isset($node->field_analysis_collaboration[LANGUAGE_NONE][0]['value'])) print $node->field_analysis_collaboration[LANGUAGE_NONE][0]['value']; ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('E-Print'); ?>:</div><div class="field-items"><?php if (isset($node->field_analysis_eprint[LANGUAGE_NONE][0]['url']))
 print l( $node->field_analysis_eprint[LANGUAGE_NONE][0]['title'], $node->field_analysis_eprint[LANGUAGE_NONE][0]['url']); ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Journal'); ?>:</div><div class="field-items"><?php if (isset($node->field_analysis_journal[LANGUAGE_NONE][0]['value'])) print $node->field_analysis_journal[LANGUAGE_NONE][0]['value']; ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('inSpire URL'); ?>:</div><div class="field-items"><?php if (isset($node->field_analysis_inspire[LANGUAGE_NONE][0]['url'])) print l( $node->field_analysis_inspire[LANGUAGE_NONE][0]['title'], $node->field_analysis_inspire[LANGUAGE_NONE][0]['url']); ?></div></div>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('DOI'); ?>:</div><div class="field-items"><?php if (isset($node->field_analysis_doi[LANGUAGE_NONE][0]['value'])) print $node->field_analysis_doi[LANGUAGE_NONE][0]['value']; ?></div></div>
  <?php if (isset($node->field_analysis_description[LANGUAGE_NONE][0]['value'])) { ?>
  <div class="field-label-inline field field-name-field-analysis-description clearfix"><div class="field-label"><?php print t('Description'); ?>:</div><div class="field-items"><?php print $node->field_analysis_description[LANGUAGE_NONE][0]['value']; ?></div></div>
  <?php } else { ?>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Description'); ?>:</div></div>
  <?php } ?>
  <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Owner'); ?>:
  </div><div class="field-items">
    <?php
      if (isset($node->field_analysis_owner[LANGUAGE_NONE][0]['uid'])) {
        $user = user_load($node->field_analysis_owner[LANGUAGE_NONE][0]['uid']); print $user->name;
      } else if (user_is_logged_in()) { ?>
        <form action="/" id="claim_ownership_form">
          <input type="hidden" name="analysis_nid" value="<?php print $node->nid; ?>">
          <input type="submit" value="Claim Ownership" />
        </form>
        <div id="claim_ownership_message" style="display:none;">An email has been sent to the site administrator for your claim request</div>
      <?php } else {
        print t('Unclaimed');
      }
    ?>
  </div></div>
  <?php
    if (isset($content['field_run_condition'])) {
      ?><fieldset class="fieldset"><legend><?php print t('Run Conditions'); ?></legend>
      <div class="fieldset-wrapper">
      <?php
      $runconditions = 0;
      foreach ($content['field_run_condition'] as $key => $item) {
        if (is_int($key) AND isset($item['entity'])) {
          $runconditions++;
          $run_condition = current($item['entity']['field_collection_item']);
          ?>
          <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Name'); ?>:</div><div class="field-items"><?php print $run_condition['field_run_condition_name']['#items'][0]['value']; ?></div></div>
          <div class="field-label-inline field clearfix"><div class="field-label"><?php print t('Description'); ?>:</div><div class="field-items"><?php print $run_condition['field_run_condition_description']['#items'][0]['value']; ?></div></div>
          <?php
        }
      }
      if ($runconditions == 0) {
          print t('Non Defined');
      }
      ?></div></fieldset><?php
    }
  ?>

</div>

<script type="text/javascript">

(function ($) {

  Drupal.behaviors.exampleModule = {
    attach: function (context, settings) {
      $('#claim_ownership_form').submit(function(event) {
        /* stop form from submitting normally */
        event.preventDefault();

        /* get some values from elements on the page: */
        var $form = $( this ),
          analysis_nid = $form.find( 'input[name="analysis_nid"]' ).val();

        $.ajax( {
          type : 'POST',
          cache : false,
          url : "recast/claim_ownership",
          data:  { nid: analysis_nid },
          dataType : "json",
          success : function (data) {
            if (data.status == "0") { // success
              $("#claim_ownership_form").hide();
              $("#claim_ownership_message").show();
            }
          },
          error : function (request, status, error){
            alert('Error claiming ownership');
          }
        });

      })

    }
  };

})(jQuery);

</script>