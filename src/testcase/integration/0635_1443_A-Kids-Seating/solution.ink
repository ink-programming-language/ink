// Translated from solution.cpp.

var pr = cpp_array(405);

func prime(n: dynamic)
{
  {
    var i = 2;
    while ((i < (n * n)))
    {
      if (((n % i) == 0))
      {
        pr[n].push_back(i);
        while (((n % i) == 0))
        {
          n /= i;
        }
      }
      i += 1;
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var cnt = 0;
    var b = 0;
    var ans: dynamic;
    {
      var i = (4 * n);
      while ((i >= 4))
      {
        if ((cnt == n))
        {
          break;
        }
        ans.push_back(i);
        cnt += 1;
        i -= 2;
      }
    }
    for (var x in ans)
    {
      write(x, " ");
    }
    write("\n");
  }
  return 0;
}
