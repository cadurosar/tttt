import pyterrier as pt
if not pt.started():
  pt.init()
from pyterrier_pisa import PisaIndex

# from a dataset
dataset = pt.get_dataset('irds:msmarco-passage')
index = PisaIndex('./msmarco-passage-pisa')
index.index(dataset.get_corpus_iter())
