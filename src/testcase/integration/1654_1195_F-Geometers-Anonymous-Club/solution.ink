// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = (((((x << 1)) + ((x << 3))) + c) - cpp_char("0"));
    c = getchar();
  }
  return (x * f);
}

var id: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_array(300010);

var las: dynamic = cpp_array(300010);

var q: dynamic = cpp_array(300010);

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  if ((a == 0))
  {
    return b;
  }
  return gcd(b, (a % b));
}

func Abs(x: dynamic) -> dynamic
{
  return  ((x >= 0)) ? x : (-x);
}

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point(x: dynamic = 0, y: dynamic = 0) -> dynamic
  {
      x = x;
      y = y;
    }
  func operator_subtract(b: dynamic) -> dynamic
  {
      return Point((x - b.x), (y - b.y));
    }
}

var p: dynamic = cpp_array(300010);

var t: dynamic = cpp_array(300010);

class BIT
{
  var b: dynamic = cpp_array(300010);
  func lowbit(x: dynamic) -> dynamic
  {
      return (x & ((-x)));
    }
  func Add(x: dynamic, d: dynamic) -> dynamic
  {
      while ((x <= n))
      {
        b[x] += d;
        x += lowbit(x);
      }
    }
  func Ask(x: dynamic) -> dynamic
  {
      var ans: dynamic = 0;
      while (x)
      {
        ans += b[x];
        x -= lowbit(x);
      }
      return ans;
    }
}

var B: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  n = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var k: dynamic = read();
      while (cpp_update(k, "--"))
      {
        var x: dynamic = read();
        var y: dynamic = read();
        p[i].push_back(Point(x, y));
      }
      {
        var j: dynamic = 0;
        while ((j < cpp_cast(p[i].size())))
        {
          var a: dynamic = (p[i][j] - p[i][(((j + 1)) % p[i].size())]);
          var g: dynamic = gcd(Abs(a.x), Abs(a.y));
          a.x /= g;
          a.y /= g;
          if ((!id.count(make_pair(a.x, a.y))))
          {
            id[make_pair(a.x, a.y)] = cpp_update(tot, "++");
          }
          t[i].push_back(id[make_pair(a.x, a.y)]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  Q = read();
  {
    var i: dynamic = 1;
    while ((i <= Q))
    {
      var l: dynamic = read();
      var r: dynamic = read();
      q[r].emplace_back(l, i);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      for (var x: dynamic in t[i])
      {
        if (las[x])
        {
          B.Add(las[x], -1);
        }
        B.Add(i, 1);
        las[x] = i;
      }
      for (var __cpp_item_1: dynamic in q[i])
      {
        var (l, id): dynamic = __cpp_item_1;
        ans[id] = (B.Ask(i) - B.Ask((l - 1)));
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= Q))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
