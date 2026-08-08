// Translated from solution.cpp.

func hehe()
{
  var sum: dynamic;
  var limit: dynamic;
  read(sum, limit);
  var ans: dynamic;
  {
    var i = limit;
    while ((i > 0))
    {
      var j = i;
      var count = 0;
      while (((j % 2) == 0))
      {
        count += 1;
        j = (j / 2);
      }
      var p = pow(2, count);
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
    var i = 0;
    while ((i < ans.size()))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  write("\n");
}

func main()
{
  var t = 1;
  while (cpp_update(t, "--"))
  {
    hehe();
  }
  return 0;
}
