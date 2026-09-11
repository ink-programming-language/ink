// Translated from solution.cpp.

var a1: dynamic = [1, 2, 4, 7, 12, 20, 29, 38, 52, 101];

var a2: dynamic = [1, 2, 4, 7, 12, 20, 30, 39, 67, 101];

var n: dynamic = cpp_uninitialized();

var an: dynamic = cpp_array(15, 15);

var no: dynamic = 1;

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      an[i][i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = (i + 1);
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
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
