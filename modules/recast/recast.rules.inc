<?php
  /**
* Implementation of hook_rules_event_info().
*/
function recast_rules_event_info() {
  return array(
    /* Fire an rules event when someone requests ownership of an analysis */
    'recast_claim_analysis' => array(
      'label' => t('Request to claim an analysis'),
      'module' => 'recast',
      'variables' => array(
        'analysis' => array('type' => 'node', 'label' => t('The analysis.')),
      ),
    ),
  );
}

