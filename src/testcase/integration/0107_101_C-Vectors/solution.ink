// Translated from solution.cpp.

var ax: dynamic;

var ay: dynamic;

var bx: dynamic;

var by: dynamic;

var cx: dynamic;

var cy: dynamic;

var AX = cpp_array(4);

var AY = cpp_array(4);

var CX = cpp_array(4);

var CY = cpp_array(4);

func judge_51nod(ax: dynamic, ay: dynamic, cx: dynamic, cy: dynamic)
{
  var zi1 = abs(((((bx - ax)) * cx) + (((by - ay)) * cy)));
  var zi2 = abs(((((by - ay)) * cx) - (((bx - ax)) * cy)));
  var mu = abs(((cx * cx) + (cy * cy)));
  if (mu)
  {
    if (((zi1 % mu) || (zi2 % mu)))
    {
      return 0;
    }
  }
  return 1;
}

func judge(ax: dynamic, ay: dynamic, cx: dynamic, cy: dynamic)
{
  if (((cx == 0) && (cy == 0)))
  {
    return ((ax == bx) && (ay == by));
  }
  var zi1 = abs(((((bx - ax)) * cx) + (((by - ay)) * cy)));
  var zi2 = abs(((((by - ay)) * cx) - (((bx - ax)) * cy)));
  var mu = abs(((cx * cx) + (cy * cy)));
  return (((zi1 % mu) == 0) && ((zi2 % mu) == 0));
}

func main()
{
  read(ax, ay, bx, by, cx, cy);
  AX[0] = ax;
  AY[0] = ay;
  AX[1] = (-ay);
  AY[1] = ax;
  AX[2] = (-ax);
  AY[2] = (-ay);
  AX[3] = ay;
  AY[3] = (-ax);
  CX[0] = cx;
  CY[0] = cy;
  CX[1] = (-cy);
  CY[1] = cx;
  CX[2] = (-cx);
  CY[2] = (-cy);
  CX[3] = cy;
  CY[3] = (-cx);
  {
    var i = 0;
    while ((i < 4))
    {
      {
        var j = 0;
        while ((j < 4))
        {
          if (judge(AX[i], AY[i], CX[j], CY[j]))
          {
            return cpp_comma((cout << "YES"), 0);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write("NO");
}
