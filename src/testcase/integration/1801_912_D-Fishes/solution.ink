// Translated from solution.cpp.

var mod: dynamic = 998244353;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var xx: dynamic = [1, -1, 0, 0];

var yy: dynamic = [0, 0, 1, -1];

var stx: dynamic = cpp_uninitialized();

var sty: dynamic = cpp_uninitialized();

var num: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100005);

var b: dynamic = cpp_array(100005);

var ans: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

func giv(x: dynamic, y: dynamic) -> dynamic
{
  return (a[x] * b[y]);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(n, m, r, k);
  stx = (((n + 1)) / 2);
  sty = (((m + 1)) / 2);
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
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
    var p: dynamic = q.top();
    q.pop();
    var x: dynamic = p.second.first;
    var y: dynamic = p.second.second;
    ans += p.first;
    num += 1;
    if ((num == k))
    {
      break;
    }
    mp[[x, y]] = 1;
    {
      var i: dynamic = 0;
      while ((i < 4))
      {
        var nx: dynamic = (x + xx[i]);
        var ny: dynamic = (y + yy[i]);
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
  var x: dynamic = ((((n - r) + 1)) * (((m - r) + 1)));
  write(fixed, setprecision(10), (ans / x));
}
