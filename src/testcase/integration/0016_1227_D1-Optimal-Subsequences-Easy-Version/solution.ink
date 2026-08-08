// Translated from solution.cpp.

func main(argc: dynamic, argv: dynamic)
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  var v2 = v;
  sort(v.rbegin(), v.rend());
  var m: dynamic;
  read(m);
  {
    var i = 0;
    while ((i < m))
    {
      var k: dynamic;
      var ind: dynamic;
      read(k, ind);
      var kmax: dynamic;
      {
        var i = 0;
        while ((i < k))
        {
          kmax.insert(v[i]);
          i += 1;
        }
      }
      var seq: dynamic;
      {
        var i = 0;
        while ((i < n))
        {
          var it = kmax.find(v2[i]);
          if ((it != kmax.end()))
          {
            kmax.erase(it);
            seq.push_back(v2[i]);
          }
          if ((seq.size() == k))
          {
            break;
          }
          i += 1;
        }
      }
      write(seq[(ind - 1)], "\n");
      i += 1;
    }
  }
  return 0;
}
