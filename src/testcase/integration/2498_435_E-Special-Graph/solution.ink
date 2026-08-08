// Translated from solution.cpp.

var s = cpp_array(1111, 1111);

var a = cpp_array(1111, 1111);

var b = cpp_array(1111, 1111);

var m: dynamic;

var n: dynamic;

func init(argument_0: dynamic)
{
  scanf("%d%d", (&m), (&n));
  {
    var i = 0;
    while ((i < (m)))
    {
      scanf("%s", s[i]);
      i = (i + 1);
    }
  }
  {
    var i = 0;
    while ((i < (m)))
    {
      {
        var j = 0;
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

func okrow(s: dynamic)
{
  {
    var i = 0;
    while ((i < (m)))
    {
      var v: dynamic;
      {
        var j = 0;
        while ((j < (4)))
        {
          if (((((((s) >> (j))) & 1)) == (i % 2)))
          {
            v.push_back(j);
          }
          j = (j + 1);
        }
      }
      var fix = false;
      {
        var j = 0;
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
        var j = 0;
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
    var i = 0;
    while ((i < (m)))
    {
      {
        var j = 0;
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

func okcol(s: dynamic)
{
  {
    var j = 0;
    while ((j < (n)))
    {
      var v: dynamic;
      {
        var i = 0;
        while ((i < (4)))
        {
          if (((((((s) >> (i))) & 1)) == (j % 2)))
          {
            v.push_back(i);
          }
          i = (i + 1);
        }
      }
      var fix = false;
      {
        var i = 0;
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
        var i = 0;
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
    var i = 0;
    while ((i < (m)))
    {
      {
        var j = 0;
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

func process(argument_0: dynamic)
{
  {
    var i = 0;
    while ((i < (4)))
    {
      {
        var j = ((i + 1));
        while ((j <= (3)))
        {
          var s = (((1 << i)) | ((1 << j)));
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

func main(argument_0: dynamic)
{
  init();
  process();
  return 0;
}
