// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var ant: dynamic;
  scanf("%d", (&ant));
  var max_mes = 0;
  {
    var i = 1;
    while ((i < n))
    {
      var aux: dynamic;
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
