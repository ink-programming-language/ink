// Translated from solution.cpp.

var a1234: dynamic;

func xxx()
{
  {
    while (true)
    {
    }
  }
}

func rd(l: dynamic, r: dynamic)
{
  return ((rand() % (((r - l) + 1))) + l);
}

var mxn = (1e5 + 3);

var a = cpp_array(mxn);

var now = cpp_array(mxn);

var n: dynamic;

var m: dynamic;

var k: dynamic;

var hh: dynamic;

var q: dynamic;

var day = cpp_array(mxn);

var h = cpp_array(mxn);

func ins(x: dynamic)
{
  q.push(pair((-((now[x] / a[x]))), x));
}

func work(x: dynamic)
{
  q = priority_queue();
  {
    var i = 1;
    while ((i <= n))
    {
      now[i] = x;
      ins(i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var re = k;
      var x: dynamic;
      var y: dynamic;
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
            var j = 1;
            while ((j <= n))
            {
              now[j] -= (m * a[j]);
              assert((now[j] >= 0));
              j += 1;
            }
          }
          re += (((m - i)) * k);
          {
            var j = 1;
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
    var i = 1;
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

func main()
{
  a1234 = scanf("%d%d%d%d", (&n), (&m), (&k), (&hh));
  var l = 0;
  var r = 0;
  var mid: dynamic;
  {
    var i = 1;
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
