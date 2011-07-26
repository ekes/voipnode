// hide disabled scripts from admin options form
Drupal.behaviors.voipnodeAdminOptions = function (context) {
  $('select#edit-voipnode-default-script-name option').each(function() {
    var $thisOption = $(this);
    var $thisValue = this.value;
    var $thisCheckBox = $('input[@value='+$thisValue+']');
    if (! $thisCheckBox.is(':checked')) {
      $thisOption.hide();
      $thisOption.removeAttr('selected');
    }
    $thisCheckBox.click(function() {
      if (this.checked) {
        $thisOption.show();
      }
      else {
        $thisOption.hide();
        $thisOption.removeAttr('selected');
      }
    });
  });
}
