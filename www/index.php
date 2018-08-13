<?php
$filename = 'data.json';
$handle = fopen($filename, 'r') or die('Cannot open file:  ' . $filename);
$data = json_decode(fread($handle,filesize($filename)));
fclose($handle);

function teamToColor($team) {
  $color;
  switch ($team) {
    case 'rood':
      $color = '#ffc9c9';
      break;
    case 'groen':
      $color ='#cdfecd';
      break;
    case 'blauw':
      $color = '#c8c8fc';
      break;
  }
  return $color;
}
?>
  <!doctype html>
  <html lang="nl">

  <head>
    <!-- Required meta tags -->
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">

    <!-- Bootstrap CSS -->
    <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css" integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO"
      crossorigin="anonymous">

    <title>Capture The Fri3d</title>
  </head>

  <body>
    <div class="container">
      <h1>Capture the Fri3d</h1>
      <div class="row">
        <div class="col">
          <h2>Scores</h2>
          <ul>
            <?php foreach ($data->scores as $team=>$score): ?>
            <li>team
              <?php echo $team; ?>:
              <?php echo $score; ?>
            </li>
            <?php endforeach; ?>
          </ul>
        </div>
      </div>
      <div class="row">
        <div class="col">
          <h2>Torens</h2>
          <div class="row">
              <?php foreach ($data->towers as $tower=>$towerData): ?>
              <div class="col" style="margin: 5px; text-align: center; background-color: <?php echo teamToColor($towerData->currentLeadingTeam); ?>">
                <h3>
                  <?php echo $towerData->name; ?>
                </h3>
                <div>
                  <strong>leider</strong>:
                  <?php echo $towerData->currentLeadingTeam; ?>
                </div>
                <div class="row">
                  <?php foreach ($towerData->teamData as $team=>$teamData): ?>
                  <div class="col" style="text-align: center; background-color: <?php echo teamToColor($teamData->teamId); ?>">
                    <div style="text-align: center;">
                    <?php echo $teamData->teamId; ?><br />
                    <?php echo $teamData->teamCount; ?>
                    </div>
                  </div>
                  <?php endforeach; ?>
                </div>
              </div>
              <?php endforeach; ?>
            
          </div>
        </div>
      </div>
    </div>
    <!-- Optional JavaScript -->
    <!-- jQuery first, then Popper.js, then Bootstrap JS -->
    <script src="https://code.jquery.com/jquery-3.3.1.slim.min.js" integrity="sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo"
      crossorigin="anonymous"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.3/umd/popper.min.js" integrity="sha384-ZMP7rVo3mIykV+2+9J3UJ46jBk0WLaUAdn689aCwoqbBJiSnjAK/l8WvCWPIPm49"
      crossorigin="anonymous"></script>
    <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js" integrity="sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy"
      crossorigin="anonymous"></script>
  </body>

  </html>