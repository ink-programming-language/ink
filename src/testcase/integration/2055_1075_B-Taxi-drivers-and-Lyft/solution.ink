// Translated from solution.cpp.

var drivers: dynamic = cpp_uninitialized();

var residents: dynamic = cpp_uninitialized();

var arr: dynamic = cpp_array(200009);

var ans: dynamic = cpp_array(200009);

func lbound(x: dynamic, m: dynamic) -> dynamic
{
  var lo: dynamic = 0;
  var hi: dynamic = (m - 1);
  var idx: dynamic = -1;
  while ((lo <= hi))
  {
    var mid: dynamic = (((lo + hi)) / 2);
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

func ubound(x: dynamic, m: dynamic) -> dynamic
{
  var lo: dynamic = 0;
  var hi: dynamic = (m - 1);
  var idx: dynamic = -1;
  while ((lo <= hi))
  {
    var mid: dynamic = (((lo + hi)) / 2);
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < (n + m)))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var dc: dynamic = 0;
  var rc: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (n + m)))
    {
      var c: dynamic = cpp_uninitialized();
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
  var minm: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var lbi: dynamic = lbound(residents[i].first, m);
      var ubi: dynamic = ubound(residents[i].first, m);
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
    var i: dynamic = 0;
    while ((i < m))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  write("\n");
}
