// Translated from solution.cpp.

var vis: dynamic = cpp_array(200009);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var i: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_array(n);
  var r: dynamic = cpp_array(n);
  {
    i = 0;
    while ((i < n))
    {
      read(l[i], r[i]);
      i += 1;
    }
  }
  var diff: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < (n - 1)))
    {
      diff.push_back(make_pair(make_pair((r[(i + 1)] - l[i]), (l[(i + 1)] - r[i])), i));
      i += 1;
    }
  }
  var bridge: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < m))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      bridge.insert([x, (i + 1)]);
      i += 1;
    }
  }
  if ((m < (n - 1)))
  {
    write("No");
    return 0;
  }
  sort(diff.begin(), diff.end());
  for (var p: dynamic in diff)
  {
    var le: dynamic = p.first.first;
    var ri: dynamic = p.first.second;
    var x: dynamic = p.second;
    var it: dynamic = bridge.upper_bound(make_pair(ri, -1));
    if (((it == bridge.end()) || (it->first > le)))
    {
      write("No");
      return 0;
    }
    ans[x] = it->second;
    bridge.erase(it);
  }
  write("Yes\n");
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}
