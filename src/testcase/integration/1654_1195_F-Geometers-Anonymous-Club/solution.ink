// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var c = getchar();
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

var id: dynamic;

var n: dynamic;

var tot: dynamic;

var Q: dynamic;

var ans = cpp_array(300010);

var las = cpp_array(300010);

var q = cpp_array(300010);

func gcd(a: dynamic, b: dynamic)
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

func Abs(x: dynamic)
{
  return if ((x >= 0)) x else (-x);
}

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point(x: dynamic = 0, y: dynamic = 0)
  {
      x = x;
      y = y;
    }
  func operator_subtract(b: dynamic)
  {
      return Point((x - b.x), (y - b.y));
    }
}

var p = cpp_array(300010);

var t = cpp_array(300010);

class BIT
{
  var b: dynamic = cpp_array(300010);
  func lowbit(x: dynamic)
  {
      return (x & ((-x)));
    }
  func Add(x: dynamic, d: dynamic)
  {
      while ((x <= n))
      {
        b[x] += d;
        x += lowbit(x);
      }
    }
  func Ask(x: dynamic)
  {
      var ans = 0;
      while (x)
      {
        ans += b[x];
        x -= lowbit(x);
      }
      return ans;
    }
}

var B: dynamic;

func main()
{
  n = read();
  {
    var i = 1;
    while ((i <= n))
    {
      var k = read();
      while (cpp_update(k, "--"))
      {
        var x = read();
        var y = read();
        p[i].push_back(Point(x, y));
      }
      {
        var j = 0;
        while ((j < cpp_cast(p[i].size())))
        {
          var a = (p[i][j] - p[i][(((j + 1)) % p[i].size())]);
          var g = gcd(Abs(a.x), Abs(a.y));
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
    var i = 1;
    while ((i <= Q))
    {
      var l = read();
      var r = read();
      q[r].emplace_back(l, i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      for (var x in t[i])
      {
        if (las[x])
        {
          B.Add(las[x], -1);
        }
        B.Add(i, 1);
        las[x] = i;
      }
      for (var __cpp_item_1 in q[i])
      {
        var (l, id) = __cpp_item_1;
        ans[id] = (B.Ask(i) - B.Ask((l - 1)));
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= Q))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
