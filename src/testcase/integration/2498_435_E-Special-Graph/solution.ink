// Translated from solution.cpp.

var s: dynamic = cpp_array(1111, 1111);

var a: dynamic = cpp_array(1111, 1111);

var b: dynamic = cpp_array(1111, 1111);

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func init(argument_0: dynamic) -> dynamic
{
  scanf("%d%d", (&m), (&n));
  {
    var i: dynamic = 0;
    while ((i < (m)))
    {
      scanf("%s", s[i]);
      i = (i + 1);
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (m)))
    {
      {
        var j: dynamic = 0;
        while ((j < (n)))
        {
          a[i][j] = (s[i][j] - 49);
          j = (j + 1);
        }
      }
      i = (i + 1);
    }
  }
}

func okrow(s: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < (m)))
    {
      var v: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
        while ((j < (4)))
        {
          if (((((((s) >> (j))) & 1)) == (i % 2)))
          {
            v.push_back(j);
          }
          j = (j + 1);
        }
      }
      var fix: dynamic = false;
      {
        var j: dynamic = 0;
        while ((j < (n)))
        {
          if ((a[i][j] < 0))
          {
            j = (j + 1);
            continue;
          }
          if (((((((s) >> (a[i][j]))) & 1)) != (i % 2)))
          {
            return (false);
          }
          if ((fix && (a[i][j] != v[(j % 2)])))
          {
            return (false);
          }
          fix = true;
          if ((a[i][j] != v[(j % 2)]))
          {
            reverse(v.begin(), v.end());
          }
          j = (j + 1);
        }
      }
      {
        var j: dynamic = 0;
        while ((j < (n)))
        {
          b[i][j] = v[(j % 2)];
          j = (j + 1);
        }
      }
      i = (i + 1);
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (m)))
    {
      {
        var j: dynamic = 0;
        while ((j < (n)))
        {
          printf("%d", (b[i][j] + 1));
          j = (j + 1);
        }
      }
      printf("\n");
      i = (i + 1);
    }
  }
  return (true);
}

func okcol(s: dynamic) -> dynamic
{
  {
    var j: dynamic = 0;
    while ((j < (n)))
    {
      var v: dynamic = cpp_uninitialized();
      {
        var i: dynamic = 0;
        while ((i < (4)))
        {
          if (((((((s) >> (i))) & 1)) == (j % 2)))
          {
            v.push_back(i);
          }
          i = (i + 1);
        }
      }
      var fix: dynamic = false;
      {
        var i: dynamic = 0;
        while ((i < (m)))
        {
          if ((a[i][j] < 0))
          {
            i = (i + 1);
            continue;
          }
          if (((((((s) >> (a[i][j]))) & 1)) != (j % 2)))
          {
            return (false);
          }
          if ((fix && (a[i][j] != v[(i % 2)])))
          {
            return (false);
          }
          fix = true;
          if ((a[i][j] != v[(i % 2)]))
          {
            reverse(v.begin(), v.end());
          }
          i = (i + 1);
        }
      }
      {
        var i: dynamic = 0;
        while ((i < (m)))
        {
          b[i][j] = v[(i % 2)];
          i = (i + 1);
        }
      }
      j = (j + 1);
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (m)))
    {
      {
        var j: dynamic = 0;
        while ((j < (n)))
        {
          printf("%d", (b[i][j] + 1));
          j = (j + 1);
        }
      }
      printf("\n");
      i = (i + 1);
    }
  }
  return (true);
}

func process(argument_0: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < (4)))
    {
      {
        var j: dynamic = ((i + 1));
        while ((j <= (3)))
        {
          var s: dynamic = (((1 << i)) | ((1 << j)));
          if (okrow(s))
          {
            return;
          }
          if (okcol(s))
          {
            return;
          }
          j = (j + 1);
        }
      }
      i = (i + 1);
    }
  }
  printf("0");
}

func main(argument_0: dynamic) -> dynamic
{
  init();
  process();
  return 0;
}
