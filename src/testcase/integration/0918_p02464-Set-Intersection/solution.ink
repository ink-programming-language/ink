// Translated from solution.cpp.

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x);
      a.insert(x);
      i += 1;
    }
  }
  read(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(x);
      b.insert(x);
      i += 1;
    }
  }
  set_intersection(all(a), all(b), inserter(c, c.end()));
  for (var i: dynamic in c)
  {
    write(i, "\n");
  }
  return 0;
}
