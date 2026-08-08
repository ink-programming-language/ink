// Translated from solution.cpp.

var N = 100;

var INFTY = ((1 << 32));

var n: dynamic;

var Graph = cpp_array(N, N);

func floyd()
{
  {
    var k = 0;
    while ((k < n))
    {
      {
        var i = 0;
        while ((i < n))
        {
          if ((Graph[i][k] == INFTY))
          {
            i += 1;
            continue;
          }
          {
            var j = 0;
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

func main()
{
  var Ne: dynamic;
  read(n, Ne);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          Graph[i][j] = (if ((i == j)) 0 else INFTY);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < Ne))
    {
      var s: dynamic;
      var t: dynamic;
      var d: dynamic;
      read(s, t, d);
      Graph[s][t] = d;
      i += 1;
    }
  }
  floyd();
  {
    var i = 0;
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
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
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
