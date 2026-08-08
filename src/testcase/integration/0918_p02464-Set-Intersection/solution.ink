// Translated from solution.cpp.

func all(x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var n: dynamic;
  var m: dynamic;
  var x: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(x);
      a.insert(x);
      i += 1;
    }
  }
  read(m);
  {
    var i = 0;
    while ((i < m))
    {
      read(x);
      b.insert(x);
      i += 1;
    }
  }
  set_intersection(all(a), all(b), inserter(c, c.end()));
  for (var i in c)
  {
    write(i, "\n");
  }
  return 0;
}
