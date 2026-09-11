// Translated from solution.cpp.

func hehe() -> dynamic
{
  var sum: dynamic = cpp_uninitialized();
  var limit: dynamic = cpp_uninitialized();
  read(sum, limit);
  var ans: dynamic = cpp_uninitialized();
  {
    var i: dynamic = limit;
    while ((i > 0))
    {
      var j: dynamic = i;
      var count: dynamic = 0;
      while (((j % 2) == 0))
      {
        count += 1;
        j = (j / 2);
      }
      var p: dynamic = pow(2, count);
      if (((sum - p) >= 0))
      {
        sum -= p;
        ans.push_back(i);
      }
      i -= 1;
    }
  }
  if (sum)
  {
    write(-1, "\n");
    return;
  }
  write(ans.size(), "\n");
  {
    var i: dynamic = 0;
    while ((i < ans.size()))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  write("\n");
}

func main() -> dynamic
{
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    hehe();
  }
  return 0;
}
