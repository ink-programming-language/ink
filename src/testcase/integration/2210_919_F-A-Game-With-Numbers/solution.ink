// Translated from solution.cpp.

func codifica(cuantos: dynamic) -> dynamic
{
  var c: dynamic = 0;
  var b: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 5))
    {
      b += cuantos[i];
      if ((i < 4))
      {
        c |= (1 << b);
        b += 1;
      }
      i += 1;
    }
  }
  return c;
}

func decodifica(c: dynamic, cuantos: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 5))
    {
      cuantos[i] = 0;
      i += 1;
    }
  }
  var i: dynamic = 0;
  {
    var b: dynamic = 0;
    while ((b < 12))
    {
      if ((c & ((1 << b))))
      {
        i += 1;
      } else
      {
        cuantos[i] += 1;
      }
      b += 1;
    }
  }
}

var cuantos: dynamic = cpp_array(5, 2);

func codifica() -> dynamic
{
  return pair(codifica(cuantos[0]), codifica(cuantos[1]));
}

func decodifica(p: dynamic) -> dynamic
{
  decodifica(p.first, cuantos[0]);
  decodifica(p.second, cuantos[1]);
}

var n: dynamic = cpp_uninitialized();

var estado: dynamic = cpp_array((1 << 12));

var gana: dynamic = cpp_array((1 << 12), (1 << 12));

var pierde: dynamic = cpp_array((1 << 12), (1 << 12));

var aridad: dynamic = cpp_array((1 << 12), (1 << 12));

var vepierde: dynamic = cpp_array((1 << 12), (1 << 12));

var q: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  {
    var c: dynamic = 0;
    while ((c < (1 << 12)))
    {
      if ((builtin_popcount(c) == 4))
      {
        estado[cpp_update(n, "++")] = c;
      }
      c += 1;
    }
  }
  {
    var i0: dynamic = 0;
    while ((i0 < n))
    {
      {
        var i1: dynamic = 0;
        while ((i1 < n))
        {
          var c0: dynamic = estado[i0];
          var c1: dynamic = estado[i1];
          decodifica(pair(c0, c1));
          {
            var a: dynamic = 0;
            while ((a < 25))
            {
              var pos0: dynamic = (a % 5);
              var pos1: dynamic = (a / 5);
              if (cpp_binary(cpp_binary(cpp_binary((pos0 > 0), "and", cuantos[0][pos0]), "and", (pos1 > 0)), "and", cuantos[1][pos1]))
              {
                aridad[c0][c1] |= (1 << a);
              }
              a += 1;
            }
          }
          if ((cuantos[1][0] == 8))
          {
            pierde[c0][c1] = 1;
            q.push(pair(c0, c1));
          }
          i1 += 1;
        }
      }
      i0 += 1;
    }
  }
  while (cpp_unary("not", q.empty()))
  {
    var p: dynamic = q.front();
    q.pop();
    decodifica(p);
    {
      var pos1: dynamic = 0;
      while ((pos1 < 5))
      {
        {
          var pos0: dynamic = 0;
          while ((pos0 < 5))
          {
            if (cpp_binary(cpp_binary(cpp_binary((pos0 > 0), "and", (pos1 > 0)), "and", (cuantos[0][pos0] != 0)), "and", (cuantos[1][(((pos0 + pos1)) % 5)] != 0)))
            {
              var nextpos1: dynamic = (((pos1 + pos0)) % 5);
              var a: dynamic = ((5 * pos0) + pos1);
              cuantos[1][pos1] += 1;
              cuantos[1][nextpos1] -= 1;
              var nextp: dynamic = codifica();
              var c0: dynamic = nextp.first;
              var c1: dynamic = nextp.second;
              swap(c0, c1);
              if (cpp_binary(cpp_unary("not", gana[c0][c1]), "and", (aridad[c0][c1] & ((1 << a)))))
              {
                aridad[c0][c1] ^= (1 << a);
                gana[c0][c1] |= pierde[p.first][p.second];
                if (cpp_binary((aridad[c0][c1] == 0), "or", gana[c0][c1]))
                {
                  pierde[c0][c1] = cpp_unary("not", gana[c0][c1]);
                  q.push(nextp);
                }
              }
              cuantos[1][pos1] -= 1;
              cuantos[1][nextpos1] += 1;
            }
            pos0 += 1;
          }
        }
        pos1 += 1;
      }
    }
  }
  var casos: dynamic = cpp_uninitialized();
  scanf("%d", (&casos));
  {
    var cas: dynamic = 0;
    while ((cas < casos))
    {
      var f: dynamic = cpp_uninitialized();
      scanf("%d", (&f));
      {
        var i: dynamic = 0;
        while ((i < 2))
        {
          {
            var j: dynamic = 0;
            while ((j < 8))
            {
              cuantos[i][j] = 0;
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < 2))
        {
          {
            var j: dynamic = 0;
            while ((j < 8))
            {
              var x: dynamic = cpp_uninitialized();
              scanf("%d", (&x));
              cuantos[i][x] += 1;
              j += 1;
            }
          }
          i += 1;
        }
      }
      var p: dynamic = codifica();
      var c0: dynamic = p.first;
      var c1: dynamic = p.second;
      if (f)
      {
        swap(c0, c1);
      }
      if (gana[c0][c1])
      {
        if (f)
        {
          printf("Bob\n");
        } else
        {
          printf("Alice\n");
        }
      } else if (pierde[c0][c1])
      {
        if (f)
        {
          printf("Alice\n");
        } else
        {
          printf("Bob\n");
        }
      } else
      {
        printf("Deal\n");
      }
      cas += 1;
    }
  }
}
