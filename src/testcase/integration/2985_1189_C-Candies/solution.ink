// Translated from solution.cpp.

var N: dynamic = 200005;

var M: dynamic = (1e9 + 7);

var A: dynamic = cpp_array(N);

var ans: dynamic = cpp_uninitialized();

func func_cpp(x: dynamic, y: dynamic) -> dynamic
{
  if ((ans.find(make_pair(x, y)) != ans.end()))
  {
    return ans[make_pair(x, y)];
  }
  if ((x == y))
  {
    return cpp_assign(ans[make_pair(x, y)], "=", make_pair(0, A[x]));
  }
  var mid: dynamic = (((x + y)) / 2);
  var p: dynamic = make_pair(0, 0);
  var p1: dynamic = func_cpp(x, mid);
  var p2: dynamic = func_cpp((mid + 1), y);
  if (((p1.second + p2.second) >= 10))
  {
    p.first += 1;
  }
  p.first += (p1.first + p2.first);
  p.second = (((p1.second + p2.second)) % 10);
  return cpp_assign(ans[make_pair(x, y)], "=", p);
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(A[i]);
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    read(x, y);
    x -= 1;
    y -= 1;
    write(func_cpp(x, y).first, "\n");
  }
  return 0;
}
