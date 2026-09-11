// Translated from solution.cpp.

var maxn: dynamic = 100005;

var qq: dynamic = cpp_array(maxn);

var q: dynamic = cpp_array(maxn);

var s: dynamic = cpp_uninitialized();

var o: dynamic = cpp_array(maxn);

var w: dynamic = cpp_array(maxn);

var z: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var qa: dynamic = cpp_uninitialized();
  var qb: dynamic = cpp_uninitialized();
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(q[i]);
      s += q[i];
      qq[i] = make_pair(q[i], i);
      {
        var j: dynamic = 0;
        while ((j < q[i]))
        {
          var t: dynamic = cpp_uninitialized();
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
    var i: dynamic = 0;
    while ((i < qa))
    {
      o[qq[i].second] = a;
      i += 1;
    }
  }
  {
    var i: dynamic = qa;
    while ((i < n))
    {
      o[qq[i].second] = b;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < m))
    {
      var k: dynamic = 0;
      for (var t: dynamic in w[i])
      {
        if ((q[t] > o[t]))
        {
          var flag: dynamic = false;
          while (1)
          {
            var it: dynamic = z.lower_bound(k);
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
    var i: dynamic = 0;
    while ((i < r.size()))
    {
      write((r[i].first.first + 1), " ", (r[i].first.second + 1), " ", (r[i].second + 1), "\n");
      i += 1;
    }
  }
}
