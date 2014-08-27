
(function ($) {
  Drupal.behaviors.recast = {
    attach: function (context, settings) {
      $('#edit-field-request-audience input').click(function() {
        if(this.value == 'all' || this.value == 'authoritative') {
          $('#edit-field-request-subscribers', context).css('display','none');
        } else {
          $('#edit-field-request-subscribers', context).css('display','');
        }
      })
    }
  };

})(jQuery);
