// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  var dp = cpp_array(35);
  dp[0] = 1;
  {
    var i = 1;
    while ((i < 35))
    {
      dp[i] = ((2 * dp[(i - 1)]) + 1);
      i += 1;
    }
  }
  var cells = cpp_array(35);
  {
    var i = 0;
    while ((i < 35))
    {
      cells[i] = ((dp[i] * ((dp[i] + 1))) / 2);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < 35))
    {
      cells[i] += cells[(i - 1)];
      i += 1;
    }
  }
  while (cpp_update(t, "--"))
  {
    var x: dynamic;
    read(x);
    var ans = 0;
    {
      var i = 0;
      while ((i < 35))
      {
        if ((cells[i] > x))
        {
          ans = i;
          break;
        }
        i += 1;
      }
    }
    write(ans, "\n");
  }
}
