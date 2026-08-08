// Translated from solution.cpp.

var maxn = 100005;

var qq = cpp_array(maxn);

var q = cpp_array(maxn);

var s: dynamic;

var o = cpp_array(maxn);

var w = cpp_array(maxn);

var z: dynamic;

var r: dynamic;

func main()
{
  var n: dynamic;
  var m: dynamic;
  var a: dynamic;
  var b: dynamic;
  var qa: dynamic;
  var qb: dynamic;
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(q[i]);
      s += q[i];
      qq[i] = make_pair(q[i], i);
      {
        var j = 0;
        while ((j < q[i]))
        {
          var t: dynamic;
          read(t);
          t -= 1;
          w[t].insert(i);
          j += 1;
        }
      }
      i += 1;
    }
  }
  a = (s / n);
  b = (a + 1);
  qb = (s % n);
  qa = (n - qb);
  sort(qq, (qq + n));
  {
    var i = 0;
    while ((i < qa))
    {
      o[qq[i].second] = a;
      i += 1;
    }
  }
  {
    var i = qa;
    while ((i < n))
    {
      o[qq[i].second] = b;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((q[i] < o[i]))
      {
        z.insert(i);
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var k = 0;
      for (var t in w[i])
      {
        if ((q[t] > o[t]))
        {
          var flag = false;
          while (1)
          {
            var it = z.lower_bound(k);
            if ((it == z.end()))
            {
              flag = true;
              break;
            }
            k = (*it);
            assert((q[k] < o[k]));
            if (w[i].count(k))
            {
              k += 1;
              continue;
            }
            r.push_back(make_pair(make_pair(t, k), i));
            q[t] -= 1;
            q[k] += 1;
            if ((q[k] == o[k]))
            {
              z.erase(k);
            }
            k += 1;
            break;
          }
          if (flag)
          {
            break;
          }
        }
      }
      i += 1;
    }
  }
  write(r.size(), "\n");
  {
    var i = 0;
    while ((i < r.size()))
    {
      write((r[i].first.first + 1), " ", (r[i].first.second + 1), " ", (r[i].second + 1), "\n");
      i += 1;
    }
  }
}
