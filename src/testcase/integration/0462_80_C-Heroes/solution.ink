// Translated from solution.cpp.

var nm: dynamic = ["", "Anka", "Chapay", "Snowy", "Hexadecimal", "Dracul", "Troll", "Cleo"];

var m: dynamic = [[0]];

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var k1: dynamic = cpp_uninitialized();

var k2: dynamic = cpp_uninitialized();

var k3: dynamic = cpp_uninitialized();

var m1: dynamic = cpp_uninitialized();

var m2: dynamic = cpp_uninitialized();

var m3: dynamic = cpp_uninitialized();

var md: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var p1: dynamic = cpp_uninitialized();

var p2: dynamic = cpp_uninitialized();

var p3: dynamic = cpp_uninitialized();

var pp1: dynamic = cpp_uninitialized();

var pp2: dynamic = cpp_uninitialized();

var aa: dynamic = [[0]];

var mm: dynamic = cpp_uninitialized();

func gen(k1: dynamic, k2: dynamic, k3: dynamic, f: dynamic = 0) -> dynamic
{
  p1 = (a / k1);
  p2 = (b / k2);
  p3 = (c / k3);
  pp1 = max(p1, max(p3, p2));
  pp2 = min(p1, min(p3, p2));
  if ((((pp1 - pp2) < md) || ((f && ((pp1 - pp2) == md)))))
  {
    md = (pp1 - pp2);
    if (f)
    {
      mm += 1;
      aa[mm][1] = k1;
      aa[mm][2] = k2;
      aa[mm][3] = k3;
    }
  }
}

func main() -> dynamic
{
  var s1: dynamic = [0];
  var s2: dynamic = [0];
  var s3: dynamic = [0];
  read(n);
  var t: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= n))
    {
      read(s1, s2, s3);
      {
        j = 1;
        while ((j <= 7))
        {
          if ((strcmp(s1, nm[j]) == 0))
          {
            break;
          }
          j += 1;
        }
      }
      {
        t = 1;
        while ((t <= 7))
        {
          if ((strcmp(s3, nm[t]) == 0))
          {
            break;
          }
          t += 1;
        }
      }
      m[j][t] = 1;
      i += 1;
    }
  }
  read(a, b, c);
  md = 1000000000;
  {
    j = 0;
    while ((j <= 4))
    {
      {
        t = 0;
        while (((j + t) <= 4))
        {
          k1 = (1 + j);
          k2 = (1 + t);
          k3 = (((1 + 4) - t) - j);
          gen(k1, k2, k3);
          gen(k1, k3, k2);
          gen(k2, k1, k3);
          gen(k2, k3, k1);
          gen(k3, k2, k1);
          gen(k3, k1, k2);
          t += 1;
        }
      }
      j += 1;
    }
  }
  {
    j = 0;
    while ((j <= 4))
    {
      {
        t = 0;
        while (((j + t) <= 4))
        {
          k1 = (1 + j);
          k2 = (1 + t);
          k3 = (((1 + 4) - t) - j);
          gen(k1, k2, k3, 1);
          gen(k1, k3, k2, 1);
          gen(k2, k1, k3, 1);
          gen(k2, k3, k1, 1);
          gen(k3, k2, k1, 1);
          gen(k3, k1, k2, 1);
          t += 1;
        }
      }
      j += 1;
    }
  }
  var mm1: dynamic = 0;
  {
    var ii: dynamic = 1;
    while ((ii <= mm))
    {
      m1 = aa[ii][1];
      m2 = aa[ii][2];
      m3 = aa[ii][3];
      var bb: dynamic = [0];
      {
        i = 1;
        while ((i <= m1))
        {
          bb[i] = 1;
          i += 1;
        }
      }
      {
        i = (m1 + 1);
        while ((i <= (m2 + m1)))
        {
          bb[i] = 2;
          i += 1;
        }
      }
      {
        i = ((m1 + m2) + 1);
        while ((i <= 7))
        {
          bb[i] = 3;
          i += 1;
        }
      }
      var k: dynamic = cpp_array(4);
      while (true)
      {
        k[3] = cpp_assign(k[1], "=", cpp_assign(k[2], "=", 0));
        {
          i = 1;
          while ((i <= 7))
          {
            {
              j = 1;
              while ((j <= 7))
              {
                if ((bb[i] == bb[j]))
                {
                  k[bb[i]] += m[i][j];
                }
                j += 1;
              }
            }
            i += 1;
          }
        }
        mm1 = max(mm1, ((k[3] + k[1]) + k[2]));
        if (!((next_permutation((bb + 1), (bb + 8)))))
        {
          break;
        }
      }
      ii += 1;
    }
  }
  write(md, cpp_char(" "), mm1);
  return 0;
}
