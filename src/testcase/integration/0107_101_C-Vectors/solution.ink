// Translated from solution.cpp.

var ax: dynamic = cpp_uninitialized();

var ay: dynamic = cpp_uninitialized();

var bx: dynamic = cpp_uninitialized();

var by: dynamic = cpp_uninitialized();

var cx: dynamic = cpp_uninitialized();

var cy: dynamic = cpp_uninitialized();

var AX: dynamic = cpp_array(4);

var AY: dynamic = cpp_array(4);

var CX: dynamic = cpp_array(4);

var CY: dynamic = cpp_array(4);

func judge_51nod(ax: dynamic, ay: dynamic, cx: dynamic, cy: dynamic) -> dynamic
{
  var zi1: dynamic = abs(((((bx - ax)) * cx) + (((by - ay)) * cy)));
  var zi2: dynamic = abs(((((by - ay)) * cx) - (((bx - ax)) * cy)));
  var mu: dynamic = abs(((cx * cx) + (cy * cy)));
  if (mu)
  {
    if (((zi1 % mu) || (zi2 % mu)))
    {
      return 0;
    }
  }
  return 1;
}

func judge(ax: dynamic, ay: dynamic, cx: dynamic, cy: dynamic) -> dynamic
{
  if (((cx == 0) && (cy == 0)))
  {
    return ((ax == bx) && (ay == by));
  }
  var zi1: dynamic = abs(((((bx - ax)) * cx) + (((by - ay)) * cy)));
  var zi2: dynamic = abs(((((by - ay)) * cx) - (((bx - ax)) * cy)));
  var mu: dynamic = abs(((cx * cx) + (cy * cy)));
  return (((zi1 % mu) == 0) && ((zi2 % mu) == 0));
}

func main() -> dynamic
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
    var i: dynamic = 0;
    while ((i < 4))
    {
      {
        var j: dynamic = 0;
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
