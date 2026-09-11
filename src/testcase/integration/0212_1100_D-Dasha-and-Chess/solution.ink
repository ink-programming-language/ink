// Translated from solution.cpp.

class point
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
}

var rooks: dynamic = cpp_array(666);

var king: dynamic = cpp_uninitialized();

var mat: dynamic = cpp_array(1000, 1000);

func move(i: dynamic, j: dynamic) -> dynamic
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

func main() -> dynamic
{
  read(king.i, king.j);
  {
    var i: dynamic = 0;
    while ((i < 1000))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < 666))
    {
      read(rooks[i].i, rooks[i].j);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 666))
    {
      mat[rooks[i].i][rooks[i].j] = 1;
      i += 1;
    }
  }
  var o1: dynamic = cpp_uninitialized();
  var o2: dynamic = cpp_uninitialized();
  var o3: dynamic = cpp_uninitialized();
  o1 = 1;
  o2 = rooks[0].i;
  o3 = rooks[0].j;
  var merkez: dynamic = false;
  var solust: dynamic = 0;
  var solalt: dynamic = 0;
  var sagust: dynamic = 0;
  var sagalt: dynamic = 0;
  var yon: dynamic = 0;
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
      var movex: dynamic = cpp_uninitialized();
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
      var movey: dynamic = cpp_uninitialized();
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
          var q: dynamic = 0;
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
        var minim: dynamic = min(min(solust, sagust), min(solalt, sagalt));
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
