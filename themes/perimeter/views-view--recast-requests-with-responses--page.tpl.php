<?php
/**
 * @file views-view.tpl.php
 * Main view template
 *
 * Variables available:
 * - $classes_array: An array of classes determined in
 *   template_preprocess_views_view(). Default classes are:
 *     .view
 *     .view-[css_name]
 *     .view-id-[view_name]
 *     .view-display-id-[display_name]
 *     .view-dom-id-[dom_id]
 * - $classes: A string version of $classes_array for use in the class attribute
 * - $css_name: A css-safe version of the view name.
 * - $css_class: The user-specified classes names, if any
 * - $header: The view header
 * - $footer: The view footer
 * - $rows: The results of the view query, if any
 * - $empty: The empty text to display if the view is empty
 * - $pager: The pager next/prev links to display, if any
 * - $exposed: Exposed widget form/info to display
 * - $feed_icon: Feed icon to display, if any
 * - $more: A link to view more, if any
 *
 * @ingroup views_templates
 */

$q = drupal_get_destination();
$q = $q['destination'];
$x = explode("?", $q);
$q = $x[0];
if(isset($user->name) && strstr($q, 'requests') !== FALSE ) {
?>
<script type="text/javascript">
function request_redirect(uri) {
  document.location = 'index.php?q=' + uri;
}
</script>
<span style=" font-weight: bold;"><?php print t('Showing:'); ?> </span>
<select onchange="request_redirect(this.value);">
<option value="requests/all"   <?php $q == 'requests/all' || $q == 'requests' ? print ' selected="true" ' : print "";  ?>><?php print t('All Requests'); ?></option>
<option value="requests/<?php print $user->name; ?>"  <?php $q == 'requests/' . $user->name ? print ' selected="true" ' : print "";  ?>><?php print t('My Requests'); ?></option>
<option value="requests/all/<?php print $user->name; ?>"  <?php $q == 'requests/all/' . $user->name ? print ' selected="true" ' : print "";  ?>><?php print t('Requests for analyses I\'m subscribed to'); ?></option>
<option value="requests-with-response/all/all/<?php print $user->name; ?>"  <?php $q == 'requests-with-response/all/all/' . $user->name ? print ' selected="true" ' : print "";  ?>><?php print t('Requests for analyses I responded to'); ?></option>
</select>

<?php
}
?>
<style type="text/css">
#edit-field-request-status-value-wrapper label {
  padding-right: 5px;
}

.view-filters select {
  font-size: 8pt;
}
</style>
<?php if ($exposed): ?>
    <div class="view-filters" style="float: right;">
      <?php print $exposed; ?>
    </div>
  <?php endif; ?>
<style type="text/css">
#edit-field-request-status-value-wrapper label {
  float: left;
  padding-top: 2px;
}

#edit-field-request-status-value-wrapper  div {
  float: right;
}
</style>
<div class="<?php print $classes; ?>">
  <?php print render($title_prefix); ?>
  <?php if ($title): ?>
    <?php print $title; ?>
  <?php endif; ?>
  <?php print render($title_suffix); ?>
  <?php if ($header): ?>
    <div class="view-header">
      <?php print $header; ?>
    </div>
  <?php endif; ?>



  <?php if ($attachment_before): ?>
    <div class="attachment attachment-before">
      <?php print $attachment_before; ?>
    </div>
  <?php endif; ?>

  <?php if ($rows): ?>
    <div class="view-content">
      <?php print $rows; ?>
    </div>
  <?php elseif ($empty): ?>
    <div class="view-empty">
      <?php print $empty; ?>
    </div>
  <?php endif; ?>

  <?php if ($pager): ?>
    <?php print $pager; ?>
  <?php endif; ?>

  <?php if ($attachment_after): ?>
    <div class="attachment attachment-after">
      <?php print $attachment_after; ?>
    </div>
  <?php endif; ?>

  <?php if ($more): ?>
    <?php print $more; ?>
  <?php endif; ?>

  <?php if ($footer): ?>
    <div class="view-footer">
      <?php print $footer; ?>
    </div>
  <?php endif; ?>

  <?php if ($feed_icon): ?>
    <div class="feed-icon">
      <?php print $feed_icon; ?>
    </div>
  <?php endif; ?>

</div><?php /* class view */ ?>
