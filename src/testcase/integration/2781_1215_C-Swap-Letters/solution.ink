// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var s1: dynamic = cpp_uninitialized();
  var s2: dynamic = cpp_uninitialized();
  read(n);
  read(s1, s2);
  var v1: dynamic = cpp_uninitialized();
  var v2: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < n))
    {
      if (((s2[i] == cpp_char("a")) && (s2[i] != s1[i])))
      {
        v1.push_back((i + 1));
      } else if (((s2[i] == cpp_char("b")) && (s2[i] != s1[i])))
      {
        v2.push_back((i + 1));
      }
      i += 1;
    }
  }
  if (((((v1.size() + v2.size())) % 2) != 0))
  {
    write("-1");
  } else
  {
    j = (v1.size() + v2.size());
    if (((v1.size() % 2) != 0))
    {
      j /= 2;
      j += 1;
      write(j, "\n");
      {
        i = 1;
        while ((i < v1.size()))
        {
          write(v1[(i - 1)], " ", v1[i], "\n");
          i += 2;
        }
      }
      {
        i = 1;
        while ((i < v2.size()))
        {
          write(v2[(i - 1)], " ", v2[i], "\n");
          i += 2;
        }
      }
      write(v1[(v1.size() - 1)], " ", v1[(v1.size() - 1)], "\n");
      write(v2[(v2.size() - 1)], " ", v1[(v1.size() - 1)]);
    } else
    {
      j /= 2;
      write(j, "\n");
      {
        i = 0;
        while ((i < v1.size()))
        {
          write(v1[i], " ", v1[(i + 1)], "\n");
          i += 2;
        }
      }
      {
        i = 0;
        while ((i < v2.size()))
        {
          write(v2[i], " ", v2[(i + 1)], "\n");
          i += 2;
        }
      }
    }
  }
}
