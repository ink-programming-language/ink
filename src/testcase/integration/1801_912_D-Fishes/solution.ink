// Translated from solution.cpp.

var mod = 998244353;

var n: dynamic;

var m: dynamic;

var r: dynamic;

var k: dynamic;

var xx = [1, -1, 0, 0];

var yy = [0, 0, 1, -1];

var stx: dynamic;

var sty: dynamic;

var num: dynamic;

var a = cpp_array(100005);

var b = cpp_array(100005);

var ans: dynamic;

var mp: dynamic;

var q: dynamic;

func giv(x: dynamic, y: dynamic)
{
  return (a[x] * b[y]);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(n, m, r, k);
  stx = (((n + 1)) / 2);
  sty = (((m + 1)) / 2);
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] += a[(i - 1)];
      if ((((i + r) - 1) <= n))
      {
        a[i] += 1;
        a[(i + r)] -= 1;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      b[i] += b[(i - 1)];
      if ((((i + r) - 1) <= m))
      {
        b[i] += 1;
        b[(i + r)] -= 1;
      }
      i += 1;
    }
  }
  q.push([giv(stx, sty), [stx, sty]]);
  while (q.size())
  {
    var p = q.top();
    q.pop();
    var x = p.second.first;
    var y = p.second.second;
    ans += p.first;
    num += 1;
    if ((num == k))
    {
      break;
    }
    mp[[x, y]] = 1;
    {
      var i = 0;
      while ((i < 4))
      {
        var nx = (x + xx[i]);
        var ny = (y + yy[i]);
        if (((((nx <= 0) || (n < nx)) || (ny <= 0)) || (m < ny)))
        {
          i += 1;
          continue;
        }
        if (mp[[nx, ny]])
        {
          i += 1;
          continue;
        }
        mp[[nx, ny]] = 1;
        q.push([giv(nx, ny), [nx, ny]]);
        i += 1;
      }
    }
  }
  var x = ((((n - r) + 1)) * (((m - r) + 1)));
  write(fixed, setprecision(10), (ans / x));
}
