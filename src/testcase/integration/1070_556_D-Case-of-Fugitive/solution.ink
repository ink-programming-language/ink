// Translated from solution.cpp.

var vis = cpp_array(200009);

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var i: dynamic;
  var l = cpp_array(n);
  var r = cpp_array(n);
  {
    i = 0;
    while ((i < n))
    {
      read(l[i], r[i]);
      i += 1;
    }
  }
  var diff: dynamic;
  {
    i = 0;
    while ((i < (n - 1)))
    {
      diff.push_back(make_pair(make_pair((r[(i + 1)] - l[i]), (l[(i + 1)] - r[i])), i));
      i += 1;
    }
  }
  var bridge: dynamic;
  {
    i = 0;
    while ((i < m))
    {
      var x: dynamic;
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
  for (var p in diff)
  {
    var le = p.first.first;
    var ri = p.first.second;
    var x = p.second;
    var it = bridge.upper_bound(make_pair(ri, -1));
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
    var i = 0;
    while ((i < (n - 1)))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}
