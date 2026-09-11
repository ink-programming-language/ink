// Translated from solution.cpp.

var a1234: dynamic = cpp_uninitialized();

func xxx() -> dynamic
{
  {
    while (true)
    {
    }
  }
}

func rd(l: dynamic, r: dynamic) -> dynamic
{
  return ((rand() % (((r - l) + 1))) + l);
}

var mxn: dynamic = (1e5 + 3);

var a: dynamic = cpp_array(mxn);

var now: dynamic = cpp_array(mxn);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var hh: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var day: dynamic = cpp_array(mxn);

var h: dynamic = cpp_array(mxn);

func ins(x: dynamic) -> dynamic
{
  q.push(pair((-((now[x] / a[x]))), x));
}

func work(x: dynamic) -> dynamic
{
  q = priority_queue();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      now[i] = x;
      ins(i);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var re: dynamic = k;
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      while (re)
      {
        x = q.top().second;
        y = (-q.top().first);
        q.pop();
        if ((y < i))
        {
          return 0;
        } else if ((y >= m))
        {
          {
            var j: dynamic = 1;
            while ((j <= n))
            {
              now[j] -= (m * a[j]);
              assert((now[j] >= 0));
              j += 1;
            }
          }
          re += (((m - i)) * k);
          {
            var j: dynamic = 1;
            while ((j <= n))
            {
              while ((re && (now[j] < h[j])))
              {
                now[j] += hh;
                re -= 1;
              }
              if ((now[j] < h[j]))
              {
                return 0;
              }
              j += 1;
            }
          }
          return 1;
        } else
        {
          while ((((now[x] - (((y + 1)) * a[x])) < 0) && re))
          {
            re -= 1;
            now[x] += hh;
          }
          ins(x);
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (((now[i] - (m * a[i])) < h[i]))
      {
        return 0;
      }
      i += 1;
    }
  }
  return 1;
}

func main() -> dynamic
{
  a1234 = scanf("%d%d%d%d", (&n), (&m), (&k), (&hh));
  var l: dynamic = 0;
  var r: dynamic = 0;
  var mid: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a1234 = scanf("%d%lld", (h + i), (a + i));
      l = max(l, a[i]);
      r = max(r, (h[i] + (m * a[i])));
      i += 1;
    }
  }
  while ((l < r))
  {
    mid = (((l + r)) >> 1);
    if (work(mid))
    {
      r = mid;
    } else
    {
      l = (mid + 1);
    }
  }
  printf("%lld\n", l);
  return 0;
}
