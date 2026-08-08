// Translated from solution.cpp.

func codifica(cuantos: dynamic)
{
  var c = 0;
  var b = 0;
  {
    var i = 0;
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

func decodifica(c: dynamic, cuantos: dynamic)
{
  {
    var i = 0;
    while ((i < 5))
    {
      cuantos[i] = 0;
      i += 1;
    }
  }
  var i = 0;
  {
    var b = 0;
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

var cuantos = cpp_array(5, 2);

func codifica()
{
  return pair(codifica(cuantos[0]), codifica(cuantos[1]));
}

func decodifica(p: dynamic)
{
  decodifica(p.first, cuantos[0]);
  decodifica(p.second, cuantos[1]);
}

var n: dynamic;

var estado = cpp_array((1 << 12));

var gana = cpp_array((1 << 12), (1 << 12));

var pierde = cpp_array((1 << 12), (1 << 12));

var aridad = cpp_array((1 << 12), (1 << 12));

var vepierde = cpp_array((1 << 12), (1 << 12));

var q: dynamic;

func main()
{
  {
    var c = 0;
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
    var i0 = 0;
    while ((i0 < n))
    {
      {
        var i1 = 0;
        while ((i1 < n))
        {
          var c0 = estado[i0];
          var c1 = estado[i1];
          decodifica(pair(c0, c1));
          {
            var a = 0;
            while ((a < 25))
            {
              var pos0 = (a % 5);
              var pos1 = (a / 5);
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
    var p = q.front();
    q.pop();
    decodifica(p);
    {
      var pos1 = 0;
      while ((pos1 < 5))
      {
        {
          var pos0 = 0;
          while ((pos0 < 5))
          {
            if (cpp_binary(cpp_binary(cpp_binary((pos0 > 0), "and", (pos1 > 0)), "and", (cuantos[0][pos0] != 0)), "and", (cuantos[1][(((pos0 + pos1)) % 5)] != 0)))
            {
              var nextpos1 = (((pos1 + pos0)) % 5);
              var a = ((5 * pos0) + pos1);
              cuantos[1][pos1] += 1;
              cuantos[1][nextpos1] -= 1;
              var nextp = codifica();
              var c0 = nextp.first;
              var c1 = nextp.second;
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
  var casos: dynamic;
  scanf("%d", (&casos));
  {
    var cas = 0;
    while ((cas < casos))
    {
      var f: dynamic;
      scanf("%d", (&f));
      {
        var i = 0;
        while ((i < 2))
        {
          {
            var j = 0;
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
        var i = 0;
        while ((i < 2))
        {
          {
            var j = 0;
            while ((j < 8))
            {
              var x: dynamic;
              scanf("%d", (&x));
              cuantos[i][x] += 1;
              j += 1;
            }
          }
          i += 1;
        }
      }
      var p = codifica();
      var c0 = p.first;
      var c1 = p.second;
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
