// Translated from solution.cpp.

var MAXN: dynamic = 111111;

var SQ: dynamic = 200;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var go: dynamic = cpp_array(MAXN);

var qq: dynamic = cpp_array(MAXN);

var gg: dynamic = cpp_array(MAXN);

var ans: dynamic = cpp_array(MAXN);

var a: dynamic = cpp_array(MAXN);

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      scanf("%d%d", (&l), (&r));
      l -= 1;
      r -= 1;
      go[r].push_back(l);
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d%d", (&x), (&y));
      x -= 1;
      y -= 1;
      qq[y].push_back(make_pair(x, i));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      a[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((!go[i].empty()))
      {
        var l: dynamic = go[i][0];
        var now: dynamic = 0;
        var nb: dynamic = 0;
        {
          nb = 0;
          while (((now + SQ) <= (l + 1)))
          {
            while (((!gg[nb].empty()) && (gg[nb].back().first >= l)))
            {
              gg[nb].pop_back();
            }
            if ((gg[nb].empty() || (gg[nb].back().second < l)))
            {
              gg[nb].push_back(make_pair(l, i));
            } else
            {
              gg[nb].back().second = i;
            }
            nb += 1;
            now += SQ;
          }
        }
        if ((l >= now))
        {
          {
            var j: dynamic = now;
            while (((j < (now + SQ)) && (j < i)))
            {
              var x: dynamic = (lower_bound(gg[nb].begin(), gg[nb].end(), make_pair((a[j] + 1), -1)) - gg[nb].begin());
              x -= 1;
              if ((x != -1))
              {
                a[j] = max(a[j], gg[nb][x].second);
              }
              j += 1;
            }
          }
          gg[nb].clear();
          {
            var j: dynamic = now;
            while ((j <= l))
            {
              if ((a[j] >= l))
              {
                a[j] = max(a[j], i);
              }
              j += 1;
            }
          }
        }
      }
      for (var e: dynamic in qq[i])
      {
        var l: dynamic = e.first;
        var b: dynamic = a[l];
        var nb: dynamic = (l / SQ);
        var x: dynamic = (lower_bound(gg[nb].begin(), gg[nb].end(), make_pair((b + 1), -1)) - gg[nb].begin());
        x -= 1;
        if ((x != -1))
        {
          b = max(b, gg[nb][x].second);
        }
        ans[e.second] = b;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      printf("%d\n", (ans[i] + 1));
      i += 1;
    }
  }
  return 0;
}
