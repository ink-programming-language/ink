// Translated from solution.cpp.

class point
{
  var i: dynamic;
  var j: dynamic;
}

var rooks = cpp_array(666);

var king: dynamic;

var mat = cpp_array(1000, 1000);

func move(i: dynamic, j: dynamic)
{
  if ((mat[(king.i + i)][(king.j + j)] == 1))
  {
    i = 0;
  }
  king.i += i;
  king.j += j;
  write(king.i, " ", king.j, "\n");
  cout.flush();
}

func main()
{
  read(king.i, king.j);
  {
    var i = 0;
    while ((i < 1000))
    {
      {
        var j = 0;
        while ((j < 1000))
        {
          mat[i][j] = 0;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 666))
    {
      read(rooks[i].i, rooks[i].j);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 666))
    {
      mat[rooks[i].i][rooks[i].j] = 1;
      i += 1;
    }
  }
  var o1: dynamic;
  var o2: dynamic;
  var o3: dynamic;
  o1 = 1;
  o2 = rooks[0].i;
  o3 = rooks[0].j;
  var merkez = false;
  var solust = 0;
  var solalt = 0;
  var sagust = 0;
  var sagalt = 0;
  var yon = 0;
  while (1)
  {
    if ((o1 <= 0))
    {
      break;
    }
    o1 -= 1;
    mat[rooks[o1].i][rooks[o1].j] = 0;
    rooks[o1].i = o2;
    rooks[o1].j = o3;
    mat[rooks[o1].i][rooks[o1].j] = 1;
    if ((!merkez))
    {
      var movex: dynamic;
      if ((king.i > 500))
      {
        movex = -1;
      } else if ((king.i < 500))
      {
        movex = 1;
      } else
      {
        movex = 0;
      }
      var movey: dynamic;
      if ((king.j > 500))
      {
        movey = -1;
      } else if ((king.j < 500))
      {
        movey = 1;
      } else
      {
        movey = 0;
      }
      if (((movex == 0) && (movey == 0)))
      {
        merkez = true;
        {
          var q = 0;
          while ((q < 666))
          {
            if (((rooks[q].i < 500) && (rooks[q].j < 500)))
            {
              solust += 1;
            } else if (((rooks[q].i < 500) && (rooks[q].j > 500)))
            {
              sagust += 1;
            } else if (((rooks[q].i > 500) && (rooks[q].j < 500)))
            {
              solalt += 1;
            } else
            {
              sagalt += 1;
            }
            q += 1;
          }
        }
        var minim = min(min(solust, sagust), min(solalt, sagalt));
        if ((minim == solust))
        {
          yon = 3;
        } else if ((minim == sagust))
        {
          yon = 2;
        } else if ((minim == solalt))
        {
          yon = 1;
        } else
        {
          yon = 0;
        }
      } else
      {
        move(movex, movey);
        read(o1, o2, o3);
        continue;
      }
    }
    if ((yon == 0))
    {
      move(-1, -1);
    } else if ((yon == 1))
    {
      move(-1, 1);
    } else if ((yon == 2))
    {
      move(1, -1);
    } else
    {
      move(1, 1);
    }
    read(o1, o2, o3);
  }
  return 0;
}
