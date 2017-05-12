
<html>
<head>
<title>Задание Санька</title>
</head>
<body>
<?php
    
    
    $base = file("../results.txt");
    if(empty($base)){
        echo "Не работает!";
    }
    
    $result=array();
    
    foreach($base as $row){
        $result[]=$row;
    }
    
    $find=("./");
    $Replace=("../");
    
    $Replace_result=str_ireplace($find,$Replace,$result);
    
    $GeneralArray=array();   
    $p=0;
    
    
    $currentItem = ''; 

    foreach($Replace_result as $item){ 

        if(substr_count($item, ".png:")==1){ 
            $p++; 
            $currentItem = trim($item, ":\n"); 
            $GeneralArray[$currentItem] = array(); 
            $s=1; 
            $p--; 
        } 
        else{ 
            $newItem = trim($item); 
            if ($newItem != "") { 
            $GeneralArray[$currentItem][] = trim($item); 
            } 
        $p--; 
        } 
        $p++; 
        } 


    function cmp($a, $b){ 
    preg_match('/ (\d+) /', $a, $matchesA); 
    preg_match('/ (\d+) /', $b, $matchesB); 
    return (int)$matchesA[1] < (int)$matchesB[1]; 
    } 

    foreach ($GeneralArray as $key => $subArray) { 
    usort($subArray, "cmp"); 
    $GeneralArray[$key] = $subArray; 
    }
    
    $z=1;
    foreach($GeneralArray as $test => $c){
        
    $findSec = (".png:");
    $ReplaceSec = (".png");
    $Replace_resultSec=str_ireplace($findSec,$ReplaceSec,$test);
    $findSe = ('/I[a-z]+.*\//');
    $ReplaceSe = (" ");
    $Replace_resultSec1=preg_replace($findSe,$ReplaceSe,$Replace_resultSec);      
    
        
        echo '<div class="Main-block">',
            '<a  href="#" data-toggle="modal" data-target="#modal-'.$z.'">',
              '<img src="'.$Replace_resultSec.'">',
              '<p>'.$Replace_resultSec1.'</p>',
            '</a>',
        '</div>';
        $z++;       

    }
    $x=1;
    foreach($GeneralArray as $test => $c){
    $findSec = (".png:");
    $ReplaceSec = (".png");
    $Replace_resultSec=str_ireplace($findSec,$ReplaceSec,$test);
    $findSec1 = ('/I[a-z]+.*\//');
    $ReplaceSec1 = (" ");
    $Replace_resultSec1=preg_replace($findSec1,$ReplaceSec1,$Replace_resultSec);    
               echo '<div class="modal" id="modal-'.$x.'">',
                    '<div class="modal-dialog">',
                      '<div class="modal-content" >',
                        '<div class="modal-header">',
                            '<span><img src="'.$Replace_resultSec.'">'.$Replace_resultSec1.'</span><br>',
                        '</div>',
                        '<div class="modal-body" style="text-align: left">';    
                            foreach ($c as $Secpic) {
                                $findSecond = ('/-- \d+ matches/');
                                $ReplaceSecond = (" ");
                                $Replace_resultSecond=preg_replace($findSecond,$ReplaceSecond,$Secpic);
                                $findSecond1 = ('/I[a-z]+.*\//');
                                $ReplaceSecond1 = (" ");
                                $Replace_resultSecond1=preg_replace($findSecond1,$ReplaceSecond1,$Secpic);
                                                
                                
                                echo '<span><img src="'.$Replace_resultSecond.'"/>'.$Replace_resultSecond1.'</span><br>';
                            }    
                echo '</div>',
                        '<div class="modal-footer">',
                          '<button class="btn btn-danger" type="button" data-toggle="modal"  data-dismiss="modal">Закрыть</button>',                 
                        '</div>',
                      '</div>',
                    '</div>',
                  '</div>';
                  $x++; 
        
    }

?>
</body>
</html>
