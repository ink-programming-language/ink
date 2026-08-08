// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var cards: dynamic;
  var size: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var m: dynamic;
      var x: dynamic;
      read(m);
      size.push_back(m);
      var aux: dynamic;
      {
        var j = 0;
        while ((j < m))
        {
          read(x);
          aux[x] = true;
          j += 1;
        }
      }
      cards.push_back(aux);
      i += 1;
    }
  }
  var flag = false;
  {
    var i = 0;
    while ((i < n))
    {
      flag = false;
      var aux: dynamic;
      {
        var j = 0;
        while ((j < n))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          }
          aux = (cards[i] & cards[j]);
          var lit = aux.count();
          if ((lit > 0))
          {
            if (((size[j] - lit) == 0))
            {
              flag = true;
              write("NO", "\n");
              break;
            } else if ((((size[i] - lit) == 0) && ((size[j] - lit) == 0)))
            {
              flag = true;
              write("NO", "\n");
              break;
            }
          }
          j += 1;
        }
      }
      if ((!flag))
      {
        write("YES", "\n");
      }
      i += 1;
    }
  }
  return 0;
}
