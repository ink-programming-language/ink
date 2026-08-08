// Translated from solution.cpp.

var a1 = [1, 2, 4, 7, 12, 20, 29, 38, 52, 101];

var a2 = [1, 2, 4, 7, 12, 20, 30, 39, 67, 101];

var n: dynamic;

var an = cpp_array(15, 15);

var no = 1;

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      an[i][i] = 0;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = (i + 1);
        while ((j <= n))
        {
          an[i][j] = cpp_assign(an[j][i], "=", (no * a1[((j - i) - 1)]));
          j += 1;
        }
      }
      no *= a2[(n - i)];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          write(an[i][j], cpp_char(" "));
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
  return 0;
}
