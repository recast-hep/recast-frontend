
(function ($) {
  Drupal.behaviors.exampleModule = {
    attach: function (context, settings) {

      $('#edit-field-subscription-type-und input').click(function() {
        if(this.value == 'observer') {
          $('#edit-field-subscription-authoritative', context).css('display','none');
          $('#edit-field-subscription-requirements', context).css('display','none');
        } else {
          $('#edit-field-subscription-authoritative', context).css('display','');
          $('#edit-field-subscription-requirements', context).css('display','');
        }
      })
    }
  };

})(jQuery);
