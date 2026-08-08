// Translated from solution.cpp.

var n: dynamic;

var i: dynamic;

var j: dynamic;

var l: dynamic;

var p: dynamic;

var nr: dynamic;

var ok: dynamic;

var okc: dynamic;

var m = cpp_array(5, 5);

var v = cpp_array(11);

func main()
{
  read(n);
  read(v);
  ok = 0;
  {
    i = 0;
    while ((i <= (n - 1)))
    {
      if ((v[i] == cpp_char("0")))
      {
        m[4][2] = 1;
      } else if ((((((v[i] - cpp_char("0"))) % 3)) == 0))
      {
        m[(((v[i] - cpp_char("0"))) / 3)][3] = 1;
      } else
      {
        m[(((((v[i] - cpp_char("0"))) / 3)) + 1)][((((v[i] - cpp_char("0"))) % 3))] = 1;
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= 3))
    {
      okc = 1;
      {
        j = 1;
        while ((j <= 3))
        {
          if ((m[i][j] == 1))
          {
            okc = 0;
          }
          j += 1;
        }
      }
      if ((okc == 1))
      {
        ok = 1;
      }
      i = (i + 2);
    }
  }
  {
    i = 1;
    while ((i <= 3))
    {
      okc = 1;
      {
        j = 1;
        while ((j <= 3))
        {
          if ((m[j][i] == 1))
          {
            okc = 0;
          }
          j += 1;
        }
      }
      if ((okc == 1))
      {
        ok = 1;
      }
      i = (i + 2);
    }
  }
  if ((((((!ok) && (m[3][1] == 0)) && (m[3][2] == 1)) && (m[3][3] == 0)) && (m[4][2] == 0)))
  {
    write("NO");
  } else if ((!ok))
  {
    write("YES");
  } else
  {
    if ((ok && (m[4][2] == 1)))
    {
      {
        i = 1;
        while ((i <= 3))
        {
          if ((m[1][i] == 1))
          {
            write("YES");
            return 0;
          }
          i += 1;
        }
      }
      write("NO");
    } else
    {
      write("NO");
    }
  }
  return 0;
}
