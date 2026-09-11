// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var ant: dynamic = cpp_uninitialized();
  scanf("%d", (&ant));
  var max_mes: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var aux: dynamic = cpp_uninitialized();
      scanf("%d", (&aux));
      if ((aux < ant))
      {
        max_mes = i;
      }
      ant = aux;
      i += 1;
    }
  }
  printf("%d\n", max_mes);
  return 0;
}
