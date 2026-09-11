// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var maxi: dynamic = 0;
  read(n);
  var store: dynamic = cpp_array((n + 1));
  var s: dynamic = cpp_uninitialized();
  var s1: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x, y);
      if ((x != -1))
      {
        store[x].push_back((i + 1));
      }
      s1[(i + 1)] = y;
      if ((y == 1))
      {
        s[x] += 1;
      }
      i += 1;
    }
  }
  var ans: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((((store[i].size() == s[i]) && (s1[i] == 1))))
      {
        ans.push_back(i);
      }
      i += 1;
    }
  }
  if ((ans.size() == 0))
  {
    write(-1, "\n");
    return 0;
  }
  sort(ans.begin(), ans.end());
  {
    var i: dynamic = 0;
    while ((i < ans.size()))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  write("\n");
  return 0;
}
