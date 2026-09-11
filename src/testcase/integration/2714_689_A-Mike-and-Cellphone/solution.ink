// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var nr: dynamic = cpp_uninitialized();

var ok: dynamic = cpp_uninitialized();

var okc: dynamic = cpp_uninitialized();

var m: dynamic = cpp_array(5, 5);

var v: dynamic = cpp_array(11);

func main() -> dynamic
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
