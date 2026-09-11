// Translated from solution.cpp.

var inf: dynamic = INT_MAX;

var inf64: dynamic = LLONG_MAX;

var vect: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var d4: dynamic = [[-1, 0], [1, 0], [0, 1], [0, -1]];

func good(xx: dynamic, yy: dynamic) -> dynamic
{
  return (cpp_assign(cpp_binary(cpp_binary((xx < n), "and", (xx >= 0)), "and", yy), "=", 0));
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    vect = vector(n);
    var i: dynamic = cpp_uninitialized();
    {
      i = 0;
      while ((i < n))
      {
        read(vect[i]);
        i += 1;
      }
    }
    var ans1: dynamic = cpp_uninitialized();
    var ans2: dynamic = cpp_uninitialized();
    for (var p: dynamic in d4)
    {
      var xx: dynamic = cpp_uninitialized();
      var yy: dynamic = cpp_uninitialized();
      xx = (0 + p.first);
      yy = (0 + p.second);
      if (cpp_binary(good(xx, yy), "and", (vect[xx][yy] == cpp_char("1"))))
      {
        ans1.push_back([xx, yy]);
      }
      xx = ((n - 1) + p.first);
      yy = ((n - 1) + p.second);
      if (cpp_binary(good(xx, yy), "and", (vect[xx][yy] == cpp_char("0"))))
      {
        ans1.push_back([xx, yy]);
      }
    }
    for (var p: dynamic in d4)
    {
      var xx: dynamic = cpp_uninitialized();
      var yy: dynamic = cpp_uninitialized();
      xx = (0 + p.first);
      yy = (0 + p.second);
      if (cpp_binary(good(xx, yy), "and", (vect[xx][yy] == cpp_char("0"))))
      {
        ans2.push_back([xx, yy]);
      }
      xx = ((n - 1) + p.first);
      yy = ((n - 1) + p.second);
      if (cpp_binary(good(xx, yy), "and", (vect[xx][yy] == cpp_char("1"))))
      {
        ans2.push_back([xx, yy]);
      }
    }
    if ((ans1.size() < ans2.size()))
    {
      write(ans1.size(), cpp_char("\n"));
      for (var x: dynamic in ans1)
      {
        write((x.first + 1), cpp_char(" "), (x.second + 1), cpp_char("\n"));
      }
    } else
    {
      write(ans2.size(), cpp_char("\n"));
      for (var x: dynamic in ans2)
      {
        write((x.first + 1), cpp_char(" "), (x.second + 1), cpp_char("\n"));
      }
    }
  }
  return 0;
}
