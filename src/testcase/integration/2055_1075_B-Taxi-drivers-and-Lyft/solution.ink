// Translated from solution.cpp.

var drivers: dynamic;

var residents: dynamic;

var arr = cpp_array(200009);

var ans = cpp_array(200009);

func lbound(x: dynamic, m: dynamic)
{
  var lo = 0;
  var hi = (m - 1);
  var idx = -1;
  while ((lo <= hi))
  {
    var mid = (((lo + hi)) / 2);
    if ((drivers[mid].first < x))
    {
      idx = mid;
      lo = (mid + 1);
    } else
    {
      hi = (mid - 1);
    }
  }
  return idx;
}

func ubound(x: dynamic, m: dynamic)
{
  var lo = 0;
  var hi = (m - 1);
  var idx = -1;
  while ((lo <= hi))
  {
    var mid = (((lo + hi)) / 2);
    if ((drivers[mid].first > x))
    {
      idx = mid;
      hi = (mid - 1);
    } else
    {
      lo = (mid + 1);
    }
  }
  return idx;
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < (n + m)))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var dc = 0;
  var rc = 0;
  {
    var i = 0;
    while ((i < (n + m)))
    {
      var c: dynamic;
      read(c);
      if (c)
      {
        drivers.push_back(make_pair(arr[i], dc));
        dc += 1;
      } else
      {
        residents.push_back(make_pair(arr[i], rc));
        rc += 1;
      }
      i += 1;
    }
  }
  sort(residents.begin(), residents.end());
  sort(drivers.begin(), drivers.end());
  var minm = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var lbi = lbound(residents[i].first, m);
      var ubi = ubound(residents[i].first, m);
      if ((lbi == -1))
      {
        ans[drivers[ubi].second] += 1;
        i += 1;
        continue;
      }
      if ((ubi == -1))
      {
        ans[drivers[lbi].second] += 1;
        i += 1;
        continue;
      }
      if (((residents[i].first - drivers[lbi].first) <= (drivers[ubi].first - residents[i].first)))
      {
        ans[drivers[lbi].second] += 1;
      } else
      {
        ans[drivers[ubi].second] += 1;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  write("\n");
}
