// Translated from solution.cpp.

var MOD = (cpp_cast(1e9) + 7);

var N = (500000 + 5);

var n: dynamic;

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point()
  {
    }
  func Point(x: dynamic, y: dynamic)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func operator_subtract(rhs: dynamic)
  {
      return Point((x - rhs.x), (y - rhs.y));
    }
  func operator_multiply(t: dynamic)
  {
      return Point((x * t), (y * t));
    }
  func operator_add(rhs: dynamic)
  {
      return Point((x + rhs.x), (y + rhs.y));
    }
}

func det(a: dynamic, b: dynamic)
{
  return ((a.x * b.y) - (a.y * b.x));
}

func det(a: dynamic, b: dynamic, c: dynamic)
{
  return det((b - a), (c - a));
}

var points = cpp_array((N + N));

func brute()
{
  var ret = 0;
  var area = 0;
  {
    var i = 0;
    while ((i < n))
    {
      area += det(points[i], points[(((i + 1)) % n)], points[0]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = (i + 2);
        while ((j < n))
        {
          if ((((i + n) - j) < 2))
          {
            j += 1;
            continue;
          }
          var a = 0;
          {
            var k = (i + 1);
            while ((k < j))
            {
              a += det(points[i], points[k], points[(k + 1)]);
              k += 1;
            }
          }
          ret += abs(((area - a) - a));
          ret %= MOD;
          j += 1;
        }
      }
      i += 1;
    }
  }
  return ((((ret % MOD) + MOD)) % MOD);
}

func work()
{
  copy(points, (points + n), (points + n));
  var area = 0;
  {
    var i = 0;
    while ((i < n))
    {
      area += det(points[i], points[(i + 1)]);
      i += 1;
    }
  }
  var a = 0;
  var d = 0;
  var b = cpp_construct(0, 0);
  var c = cpp_construct(0, 0);
  var now = 0;
  var small = 0;
  var cnt = 0;
  {
    var i = 0;
    var j = 0;
    while ((i < n))
    {
      {
        while ((j < (i + n)))
        {
          var tmp = det(points[j], points[(j + 1)], points[i]);
          if (((((1 * now) + tmp) << 1) > area))
          {
            break;
          }
          now += tmp;
          a += ((det(points[j], points[(j + 1)]) % MOD) * j);
          a %= MOD;
          b = (b + (points[(j + 1)] * j));
          b.x %= MOD;
          b.y %= MOD;
          c = (c + (points[j] * j));
          c.x %= MOD;
          c.y %= MOD;
          d += det(points[j], points[(j + 1)]);
          d %= MOD;
          j += 1;
        }
      }
      var tmp = ((-(((a + (det(b, points[i]) % MOD)) + (det(points[i], c) % MOD)))) + (j * ((d + (det(points[j], points[i]) % MOD)))));
      tmp %= MOD;
      small += tmp;
      small %= MOD;
      a -= ((det(points[i], points[(i + 1)]) % MOD) * i);
      a %= MOD;
      b = (b - (points[(i + 1)] * i));
      b.x %= MOD;
      b.y %= MOD;
      c = (c - (points[i] * i));
      c.x %= MOD;
      c.y %= MOD;
      d -= det(points[i], points[(i + 1)]);
      d %= MOD;
      if (((now * 2) == area))
      {
        cnt += 1;
      }
      now -= det(points[i], points[(i + 1)], points[j]);
      i += 1;
    }
  }
  return (((((((((((((n * 1) * ((n - 3))) / 2) % MOD) * ((area % MOD))) % MOD) - (small * 2)) + (((((cnt / 2) * 1) * ((area % MOD)))) % MOD))) % MOD) + MOD)) % MOD);
}

func main()
{
  ios.sync_with_stdio(false);
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(points[i].x, points[i].y);
      i += 1;
    }
  }
  reverse(points, (points + n));
  write(work(), "\n");
}
