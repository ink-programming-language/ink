// Translated from solution.cpp.

var N: dynamic = 100;

var INFTY: dynamic = ((1 << 32));

var n: dynamic = cpp_uninitialized();

var Graph: dynamic = cpp_array(N, N);

func floyd() -> dynamic
{
  {
    var k: dynamic = 0;
    while ((k < n))
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          if ((Graph[i][k] == INFTY))
          {
            i += 1;
            continue;
          }
          {
            var j: dynamic = 0;
            while ((j < n))
            {
              if ((Graph[k][j] == INFTY))
              {
                j += 1;
                continue;
              }
              Graph[i][j] = min(Graph[i][j], (Graph[i][k] + Graph[k][j]));
              j += 1;
            }
          }
          i += 1;
        }
      }
      k += 1;
    }
  }
}

func main() -> dynamic
{
  var Ne: dynamic = cpp_uninitialized();
  read(n, Ne);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          Graph[i][j] = ( ((i == j)) ? 0 : INFTY);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < Ne))
    {
      var s: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      var d: dynamic = cpp_uninitialized();
      read(s, t, d);
      Graph[s][t] = d;
      i += 1;
    }
  }
  floyd();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((Graph[i][i] < 0))
      {
        write("NEGATIVE CYCLE", "\n");
        return 0;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if (j)
          {
            write(" ");
          }
          if ((Graph[i][j] == INFTY))
          {
            write("INF");
          } else
          {
            write(Graph[i][j]);
          }
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
}
