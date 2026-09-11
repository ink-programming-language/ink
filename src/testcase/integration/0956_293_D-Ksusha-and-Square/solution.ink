// Translated from solution.cpp.

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point() -> dynamic
  {
    }
  func Point(x: dynamic, y: dynamic) -> dynamic
  {
      self->x = x;
      self->y = y;
    }
  func operator_add(rhs: dynamic) -> dynamic
  {
      return Point((x + rhs.x), (y + rhs.y));
    }
  func operator_subtract(rhs: dynamic) -> dynamic
  {
      return Point((x - rhs.x), (y - rhs.y));
    }
  func Dot(rhs: dynamic) -> dynamic
  {
      return (((1 * x) * rhs.x) + ((1 * y) * rhs.y));
    }
  func Crs(rhs: dynamic) -> dynamic
  {
      return (((1 * x) * rhs.y) - ((1 * y) * rhs.x));
    }
}

func operator_less(lhs: dynamic, rhs: dynamic) -> dynamic
{
  return  (((lhs.x == rhs.x))) ? ((lhs.y < rhs.y)) : ((lhs.x < rhs.x));
}

var MAXN: dynamic = 100010;

var N: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(MAXN);

var H: dynamic = cpp_array(2);

var pcx: dynamic = cpp_array(2333333);

var pcy: dynamic = cpp_array(2333333);

var cx: dynamic = (pcx + 1233333);

var cy: dynamic = (pcy + 1233333);

var cip: dynamic = cpp_uninitialized();

func Stat(e: dynamic, Min: dynamic, Max: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  var Count: dynamic = 0;
  var sumPlain: dynamic = 0;
  var sumSquare: dynamic = 0;
  var i: dynamic = cpp_uninitialized();
  {
    i = Min;
    while ((i <= Max))
    {
      ret += (e[i] * (((((Count * i) * i) + sumSquare) - ((2.0 * i) * sumPlain))));
      Count += e[i];
      sumPlain += ((e[i] * 1.0) * i);
      sumSquare += (((e[i] * 1.0) * i) * i);
      i += 1;
    }
  }
  return ret;
}

func main() -> dynamic
{
  scanf("%d", (&N));
  var i: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= N))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d%d", (&x), (&y));
      p[i] = Point(x, y);
      i += 1;
    }
  }
  sort((p + 1), ((p + N) + 1));
  var ml: dynamic = 1;
  while ((((ml < N)) && ((p[(ml + 1)].x == p[1].x))))
  {
    ml += 1;
  }
  H[0].clear();
  H[1].clear();
  H[0].push_back(p[1]);
  H[1].push_back(p[ml]);
  {
    i = (ml + 1);
    while ((i <= N))
    {
      while ((((H[0].size() > 1)) && ((((H[0].back() - H[0][(H[0].size() - 2)])).Crs((p[i] - H[0].back())) <= 0))))
      {
        H[0].pop_back();
      }
      while ((((H[1].size() > 1)) && ((((H[1].back() - H[1][(H[1].size() - 2)])).Crs((p[i] - H[1].back())) >= 0))))
      {
        H[1].pop_back();
      }
      H[0].push_back(p[i]);
      H[1].push_back(p[i]);
      i += 1;
    }
  }
  while ((((H[0].size() > 1)) && ((H[0].back().x == H[0][(H[0].size() - 2)].x))))
  {
    H[0].pop_back();
  }
  var lit: dynamic = H[0].begin();
  var rit: dynamic = H[1].begin();
  var minX: dynamic = p[1].x;
  var maxX: dynamic = p[N].x;
  var minY: dynamic = p[1].y;
  var maxY: dynamic = p[1].y;
  {
    i = 2;
    while ((i <= N))
    {
      minY = min(minY, p[i].y);
      maxY = max(maxY, p[i].y);
      i += 1;
    }
  }
  {
    i = minY;
    while ((i <= maxY))
    {
      cy[i] = 0;
      i += 1;
    }
  }
  cx[maxX] = ((H[1].back().y - H[0].back().y) + 1);
  cy[(H[1].back().y + 1)] -= 1;
  cy[H[0].back().y] += 1;
  {
    i = minX;
    while ((i <= (maxX - 1)))
    {
      if ((((lit + 1))->x <= i))
      {
        lit += 1;
      }
      if ((((rit + 1))->x <= i))
      {
        rit += 1;
      }
      var ly: dynamic = ceil(((((cpp_double((((lit + 1))->y - lit->y)) / ((((lit + 1))->x - lit->x))) * ((i - lit->x))) + lit->y) - 1e-9));
      var ry: dynamic = floor(((((cpp_double((((rit + 1))->y - rit->y)) / ((((rit + 1))->x - rit->x))) * ((i - rit->x))) + rit->y) + 1e-9));
      cx[i] = ((ry - ly) + 1);
      cy[(ry + 1)] -= 1;
      cy[ly] += 1;
      i += 1;
    }
  }
  {
    i = (minY + 1);
    while ((i <= maxY))
    {
      cy[i] += cy[(i - 1)];
      i += 1;
    }
  }
  cip = 0;
  {
    i = minX;
    while ((i <= maxX))
    {
      cip += cx[i];
      i += 1;
    }
  }
  var ans: dynamic = (Stat(cx, minX, maxX) + Stat(cy, minY, maxY));
  ans /= cip;
  ans /= (cip - 1);
  printf("%.10f\n", ans);
}
