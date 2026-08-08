// Translated from solution.cpp.

var N = 200005;

var M = (1e9 + 7);

var A = cpp_array(N);

var ans: dynamic;

func func_cpp(x: dynamic, y: dynamic)
{
  if ((ans.find(make_pair(x, y)) != ans.end()))
  {
    return ans[make_pair(x, y)];
  }
  if ((x == y))
  {
    return cpp_assign(ans[make_pair(x, y)], "=", make_pair(0, A[x]));
  }
  var mid = (((x + y)) / 2);
  var p = make_pair(0, 0);
  var p1 = func_cpp(x, mid);
  var p2 = func_cpp((mid + 1), y);
  if (((p1.second + p2.second) >= 10))
  {
    p.first += 1;
  }
  p.first += (p1.first + p2.first);
  p.second = (((p1.second + p2.second)) % 10);
  return cpp_assign(ans[make_pair(x, y)], "=", p);
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(A[i]);
      i += 1;
    }
  }
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    var x: dynamic;
    var y: dynamic;
    read(x, y);
    x -= 1;
    y -= 1;
    write(func_cpp(x, y).first, "\n");
  }
  return 0;
}
