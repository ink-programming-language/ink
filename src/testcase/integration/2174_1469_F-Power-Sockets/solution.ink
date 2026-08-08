// Translated from solution.cpp.

var ll = dynamic;

var inf = 1e9;

var v = cpp_array(200005);

var aux = cpp_array(200005);

func main()
{
  var n: dynamic;
  var k: dynamic;
  scanf("%d%d", (&n), (&k));
  var i: dynamic;
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&v[i]));
      i += 1;
    }
  }
  sort((v + 1), ((v + n) + 1));
  reverse((v + 1), ((v + n) + 1));
  aux[0] = 1;
  aux[1] = -1;
  var sum = 0;
  var j = 1;
  {
    i = 0;
    while ((i <= 200000))
    {
      sum = (sum + aux[i]);
      aux[(i + 1)] += aux[i];
      if (((sum + aux[(i + 1)]) >= k))
      {
        printf("%d\n", (i + 1));
        return 0;
      }
      while (((aux[i] > 0) && (j <= n)))
      {
        aux[i] -= 1;
        aux[(i + 2)] += 1;
        aux[((i + 2) + (v[j] / 2))] -= 1;
        aux[(i + 2)] += 1;
        aux[((i + 2) + (((v[j] - 1)) / 2))] -= 1;
        j += 1;
        sum -= 1;
      }
      i += 1;
    }
  }
  printf("-1\n");
  return 0;
}
