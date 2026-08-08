// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var n: dynamic;
  var W: dynamic;
  var MIN = 100001;
  var sum = 0;
  var p = 0;
  var pp: dynamic;
  var cost = 0;
  var k = 0;
  var min = 100001;
  read(n);
  var box: dynamic;
  var list: dynamic;
  var mark: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(W);
      box.push_back(W);
      list.push_back(W);
      mark.push_back(false);
      if ((W < MIN))
      {
        MIN = W;
      }
      i += 1;
    }
  }
  sort(list.begin(), list.end());
  while ((p < n))
  {
    if ((mark[p] == false))
    {
      sum = 0;
      k = 0;
      min = 100001;
      pp = p;
      while ((mark[pp] == false))
      {
        mark[pp] = true;
        sum += box[pp];
        k += 1;
        if ((box[pp] < min))
        {
          min = box[pp];
        }
        pp = (find(list.begin(), list.end(), box[pp]) - list.begin());
      }
      if (((sum + (((k - 2)) * min)) > ((sum + min) + (((k + 1)) * MIN))))
      {
        cost += ((sum + min) + (((k + 1)) * MIN));
      } else
      {
        cost += (sum + (((k - 2)) * min));
      }
      p += 1;
    } else
    {
      p += 1;
    }
  }
  write(cost, "\n");
  return 0;
}
