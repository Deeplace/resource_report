Drupal.behaviors.report = function(context) {
  $('#edit-param-type').change(function () {
    switch ($(this).val()) {
      case '0': // Parameter
        $('#edit-field-id-wrapper').hide();
        $('#edit-table-name-wrapper').hide();
        $('#edit-category-wrapper').show();
        $('#edit-required-wrapper').show();
        $('#edit-default-value-wrapper').show();
        $('#edit-field-size-wrapper').show();
        break;
        
      case '1': // Column
        $('#edit-field-id-wrapper').show();
        $('#edit-table-name-wrapper').show();
        $('#edit-category-wrapper').hide();
        $('#edit-required-wrapper').hide();
        $('#edit-default-value-wrapper').hide();
        $('#edit-field-size-wrapper').hide();
        break;
    }
  }).trigger('change');
};
